import numpy as np

class NetBuilder(object):

    def __init__(self, HAL, net=None):
        """Initialize NetBuilder:

        Inputs:
        =======
        HAL : (HAL object)
        net : (hal.neuromorph.graph object, default None) 
            User may provide a custom network they constructed.
            If no network is supplied, typically one will be added with a 
            call like NetManager.create_single_pool_net()
        """
        self.HAL = HAL
        self.net = net

    def create_single_pool_net(self, Y, X, tap_matrix=None, decoders=None, biases=0, gain_divs=1):
        N = Y * X

        if tap_matrix is None:
            Din = 0
            tap_matrix = np.zeros((N, 1)) # have to put something in, (N, [[]]) might work
        else:
            Din = tap_matrix.shape[1]

        if decoders is None:
            Dout = 0
        else:
            Dout = decoders.shape[0]

        from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
        net = graph.Network("net")

        # decoders are initially zero
        # we remap them later (without touching the rest of the network) using HAL.remap_weights()
        p1 = net.create_pool("p1", tap_matrix, biases=biases, gain_divisors=gain_divs, xy=(X, Y))

        if Dout > 0:
            b1 = net.create_bucket("b1", Dout)
            o1 = net.create_output("o1", Dout)
            decoder_conn = net.create_connection("c_p1_to_b1", p1, b1, decoders)
            net.create_connection("c_b1_to_o1", b1, o1, None)

        if Din > 0:
            i1 = net.create_input("i1", Din)
            net.create_connection("c_i1_to_p1", i1, p1, None)

        self.net = net
        return net

    def create_default_taps(self, Y, X, D, nominal_tap_spacing=1):
        bad_syn = self.HAL.get_calibration('synapse', 'high_syn_bias')

        if D == 1:
            tap_matrix = np.zeros((Y, X))
            for y in range(0, Y, 2 * nominal_tap_spacing):
                for x in range(0, X, 2 * nominal_tap_spacing):
                    if not bad_syn[y,x]:
                        if x < X // 2:
                            tap_matrix[y, x] = 1
                        else:
                            tap_matrix[y, x] = -1
            return tap_matrix.reshape((Y * X, 1))

        else:
            raise ValueError("only D==1 is supported for now")

    def determine_good_fmaxes(self, dac_pd, safety_margin=1.3):
        """Determine maximum input rate (fmax) for all Pools at a given dac value

        Retrieves the synaptic delay calibration, 

        Inputs:
        =======
        dac_pd (int) : which value of DAC_SYN_PD to set. This controls the synapse pulse width.
        safety_margin (float, default 1.3) : margin to allow for decode mixing
        """

        if self.net is None:
            raise ValueError("need to create a or add a net to NetManager")

        raise NotImplementedError("synaptic delay calibration not hooked in yet")

    def open_all_diff_cuts(self):
        CORE_ID = 0        
        # connect diffusor around pools
        for tile_id in range(256):
            self.HAL.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

