from functools import singledispatch, update_wrapper
from Pystorm import *
from . Resources import *


def methdispatch(func):
    """Decorator function to allow instance method dispatching
    """
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)

    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


class NetObjNode(object):
    def __init__(self, net_obj):
        """Construct the object updating the resource_map if necessary

        A NetObjNode contains the following:
            net_obj - Pystorm.Network object
            resource - Resource object
            adjacent_net_obj - List of adjacent NetObjNodes

        :param net_obj: A Pystorm.Network object (e.g. Input, Pool, Bucket, Output)

        """
        self.net_obj = net_obj
        self.resource = self.create_resources_for_(net_obj)
        self.adjacent_net_obj = []

    @methdispatch
    def create_resources_for_(self, obj):
        raise TypeError("This type isn't supported: {}".format(type(obj)))

    @create_resources_for_.register(Input)
    def _(self, inp):
        return Source(inp.get_num_dimensions())

    @create_resources_for_.register(Output)
    def _(self, output):
        return Sink(output.get_num_dimensions())

    @create_resources_for_.register(Pool)
    def _(self, pool):
        return Neurons(pool.get_num_neurons())

    @create_resources_for_.register(Bucket)
    def _(self, bucket):
        return AMBuckets(bucket.get_num_dimensions())

    @create_resources_for_.register(WeightsClass)
    def _(self, pystorm_weights):
        weight_arr = np.zeros((pystorm_weights.get_num_rows(),
                              pystorm_weights.get_num_columns()))

        for i in range(pystorm_weights.get_num_rows()):
            for j in range(pystorm_weights.get_num_columns()):
                weight_arr[i,j] = pystorm_weights.get_element(i, j)

        ret_value = MMWeights(weight_arr)

        return ret_value

    def add_adjacent_node(self, node, weight=None):
        """Add node to the list of adjacent NetObjNodes

        :param node: Adjacent NetObjNode instance
        :param weight: A Pystorm.Weight object

        ..note ::
            The Pystorm.Weight object will be used when this instance is connected to the
            node instance. See the connect method for more details.
        """
        self.adjacent_net_obj.append((weight, node))

    def get_net_obj(self):
        return self.net_obj

    def get_resource(self):
        return self.resource

    def connect(self, resources):
        """Connect the NetObjNode to it's adjacent nodes

            Update the resources list parameter, if necessary.

        :param resources: A list of Resource instances
        :return:

        What type of Pystorm.Network object connections can we expect and
        what Resource objects (and connections) must we create given the
        Pystorm.Network objects?

        The following information describes the Pystorm.Network objects
        followed by their associated Resources objects.
        Note that the first five connections are from one object to one
        object, however, connections 6 and 7 are from one object to multiple
        objects which require a different set of resources.

          Conn 1: Input -> Pool

                      Source -> TATTapPoint -> Neurons

          Conn 2: Input -> Bucket

              Source -> TATAcuumulator -> MMWeights -> AMBuckets

          Conn 3: Pool -> Bucket

             Neurons -> MMWeights -> AMBuckets

          Conn 4: Bucket -> Bucket

              AMBuckets -> TATAccumulator -> MMWeights -> AMBuckets

          Conn 5: Bucket -> Output

             AMBuckets -> Sink

         Conn 6: Bucket -> Bucket
                        -> Bucket

             AMBuckets -> TATFanout -> MMWeights -> AMBuckets
                                    -> MMWeights -> AMBuckets

         Conn 7: Bucket -> Output
                        -> Output

             AMBuckets -> TATFanout -> Sink
                                    -> Sink

        """
        number_of_adj_nodes = len(self.adjacent_net_obj)

        if number_of_adj_nodes == 0:

            return

        elif number_of_adj_nodes == 1:

            adj_node = self.adjacent_net_obj[0][1]
            adj_node_weights = self.adjacent_net_obj[0][0]
            # Conn 1: Input -> Pool
            if (type(self.get_net_obj()) == Input) and (type(adj_node.get_net_obj()) == Pool):
                num_neurons = adj_node.get_net_obj().get_num_neurons()
                # what to do if dims is odd
                dims_in = self.get_net_obj().get_num_dimensions()
                taps_per_dim = 2

                source = self.resource
                tap = TATTapPoint(np.random.randint(num_neurons,
                                                    size=(dims_in, taps_per_dim)),
                                  np.random.randint(1, size=(dims_in,
                                                             taps_per_dim)) * 2 - 1,
                                  num_neurons)
                neurons = adj_node.get_resource()

                source.Connect(tap)
                tap.Connect(neurons)

                resources.append(tap)

            # Conn 2: Input -> Bucket
            if type(self.get_net_obj()) == Input and type(adj_node.get_net_obj()) == Bucket:
                source = self.get_resource()
                tat_acc_resource = TATAccumulator(self.get_net_obj().get_num_dimensions())
                if (adj_node_weights == None):
                    msg = "Cannot connect Input to Bucket without Weights;\n" \
                          + "Assign Weights to Pystorm.Connection between Input and Bucket"
                    raise TypeError(msg)

                weight_resource = self.create_resources_for_(adj_node_weights)
                am_bucket = adj_node.get_resource()

                source.Connect(tat_acc_resource)
                tat_acc_resource.Connect(weight_resource)
                weight_resource.Connect(am_bucket)

                resources.append(tat_acc_resource)
                resources.append(weight_resource)

            # Conn 3: Pool -> Bucket
            if type(self.get_net_obj()) == Pool and type(adj_node.get_net_obj()) == Bucket:
                neurons = self.resource
                weight_resource = self.create_resources_for_(adj_node_weights)
                am_bucket = adj_node.get_resource()

                neurons.Connect(weight_resource)
                weight_resource.Connect(am_bucket)

                resources.append(weight_resource)

            # Conn 4: Bucket -> Bucket
            if type(self.get_net_obj()) == Bucket and type(adj_node.get_net_obj()) == Bucket:
                am_bucket_in = self.get_resource()
                tat_acc_resource = TATAccumulator(self.get_net_obj().get_num_dimensions())
                if (adj_node_weights == None):
                    msg = "Cannot connect Input to Bucket without Weights;\n" \
                          + "Assign Weights to Pystorm.Connection between Input and Bucket"
                    raise TypeError(msg)
                weight_resource = self.create_resources_for_(adj_node_weights)
                am_bucket_out = adj_node.get_resource()

                am_bucket_in.Connect(tat_acc_resource)
                tat_acc_resource.Connect(weight_resource)
                weight_resource.Connect(am_bucket_out)

                resources.append(tat_acc_resource)
                resources.append(weight_resource)

            # Conn 5: Bucket -> Output
            if type(self.get_net_obj()) == Bucket and type(adj_node.get_net_obj()) == Output:
                am_bucket = self.resource
                sink = adj_node.get_resource()

                am_bucket.Connect(sink)

        elif number_of_adj_nodes > 1:

            if type(self.get_net_obj()) == Bucket:
                am_bucket_in = self.get_resource()
                tat_fanout = TATFanout(self.get_net_obj().get_num_dimensions())

                am_bucket_in.Connect(tat_fanout)

                resources.append(tat_fanout)

                for adj_node in self.adjacent_net_obj:
                    adj_node = adj_node[1]
                    adj_node_weight = adj_node[0]
                    #   Conn 6: Bucket -> Bucket
                    #                  -> Bucket
                    if type(adj_node.get_net_obj()) == Bucket:
                        weight_resource = self.create_resources_for_(adj_node_weight)
                        am_bucket_out = adj_node.get_resource()

                        resources.append(weight_resource)

                        tat_fanout.Connect(weight_resource)
                        weight_resource.Connect(am_bucket_out)

                        #   Conn 7: Bucket -> Output
                        #                  -> Output
                    if type(adj_node.get_net_obj()) == Output:
                        sink = adj_node.get_net_obj().get_resource()

                        tat_fanout.Connect(sink)
