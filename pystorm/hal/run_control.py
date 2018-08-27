import time
import logging
import numpy as np

logger = logging.getLogger(__name__)

class RunControl(object):
    """Represents a higher-level set of abstractions than HAL provides for controlling a Network
    """

    def __init__(self, HAL, net):
        """
        Inputs:
        =======
        HAL : (HAL object) currently live HAL instance
        net : (hal.neuromorph.graph.Network object) currently live network instance
        """
        self.HAL = HAL
        self.net = net

    #NOTE: beginning of ideas for real-time IO support
    #def insert_in_input_queue(self, input_obj_or_DACs, times, values):
    #    # controlling a DAC value
    #    if isinstance(input_obj_or_DAC, str):
    #        pass
    #    # controlling input values
    #    else:
    #        pass

    #def commit_input_queue(self):
    #    HAL.flush()

    def run_input_sweep(self, input_vals, get_raw_spikes=True, get_outputs=True,
                        start_time=None, end_time=None, step_options=None):
        """Run a simple input sweep, return the binned output values or raw spikes

        input_vals : {input_obj : ((len-T array) times, (TxD array) rates)}
            input rates to set at times (in ns).
            Times are in reference to HAL.get_time(), as usual.
            A bit of initial padding is recommended.
        get_raw_spikes : (bool) whether or not to return raw spike data
        get_outputs : (bool) whether or not to return Output values
        start_time : begin recording data at (in FPGA time units), defaults to min input_vals time
        end_time : stop recording data at (in FPGA time units), defaults to max input_vals time
        step_options : tuple(function, step_sleep_time, [to_fill_with_outputs], [to_fill_with_times])
            instead of sleeping for the entire duration,
            can call a user-specified function periodically.
            function is called every step_sleep_time. get_fpga_time()
            will be used to stamp the times that the call actually occurred at.
            to_fill_with_outputs/times should be empty lists that are filled up

        Returns:
        ========
        output_data, spike_data : (return formats from get_array_outputs and get_binned_spikes, i.e.
                tuple({output_id : (np.array of values indexed [time_bin_idx, dimension])}, bin_times)
                    and
                tuple(pool-keyed dict of numpy arrays, array of bin times) )
            Data measured between start_time and end_time
            Uses hardware binning interval to bin spikes (HAL.upstream_ns)
        """

        if self.HAL.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")

        if step_options is not None:
            raise NotImplementedError("step_options argument isn't supported yet")

        TFUDGE = .05 # software stack jitter, turn on data collection a little before start_time, etc.
        if start_time is None:
            start_time = min([input_vals[inp][0][0] for inp in input_vals])

        if end_time is None:
            end_time = max([input_vals[inp][0][-1] for inp in input_vals])

        def start_sweep(get_raw_spikes, get_outputs):
            """Activate chip traffic"""
            if get_raw_spikes:
                self.HAL.enable_spike_recording(flush=False)
            if get_outputs:
                self.HAL.enable_output_recording(flush=False)
            self.HAL.start_traffic(flush=True)

        def end_sweep(get_raw_spikes, get_outputs, start_time, end_time):
            """"Deactivate chip traffic, and gather output spikes and tags"""
            def window_dict_of_arrays(dict_of_arrays, bin_times, start_time, end_time):
                """Clip spike and output data in dictionary of arrays to start and end time"""
                start_idx = np.searchsorted(bin_times, start_time)
                end_idx = np.searchsorted(bin_times, end_time)

                windowed_bin_times = bin_times[start_idx:end_idx]

                windowed_dict_of_arrays = {
                    obj:dict_of_arrays[obj][start_idx:end_idx, :] for obj in dict_of_arrays}

                assert(windowed_bin_times.shape[0] ==
                       windowed_dict_of_arrays[next(iter(windowed_dict_of_arrays))].shape[0])

                return windowed_dict_of_arrays, windowed_bin_times

            if get_raw_spikes:
                self.HAL.disable_spike_recording(flush=False)
            if get_outputs:
                self.HAL.disable_output_recording(flush=False)
            self.HAL.stop_traffic(flush=True)

            # use hardware binning interval
            if get_raw_spikes:
                binned_spikes, spike_bin_times = self.HAL.get_binned_spikes(self.HAL.upstream_ns)
                windowed_spikes, spike_bin_times = window_dict_of_arrays(
                    binned_spikes, spike_bin_times, start_time, end_time)

            else:
                windowed_spikes = None
                spike_bin_times = None

            if len(self.net.get_outputs()) > 0 and get_outputs:
                array_outputs, output_bin_times = self.HAL.get_array_outputs()
                windowed_outputs, output_bin_times = window_dict_of_arrays(
                    array_outputs, output_bin_times, start_time, end_time)
            else:
                windowed_outputs = None
                output_bin_times = None

            return (windowed_outputs, output_bin_times), (windowed_spikes, spike_bin_times)

        def enqueue_input_vals(input_vals):
            """Queue up input sequence in hardware"""
            now_ns = self.HAL.get_time()
            for input_obj in input_vals:
                times, rates = input_vals[input_obj]
                assert len(times) == rates.shape[0]
                assert input_obj.dimensions == rates.shape[1]
                if times[-1] < now_ns:
                    logger.warning(
                        "All input times for %s are before current time. " +
                        "Input times should be referenced to hal.get_time()", str(input_obj))

                T, D = rates.shape

                objs = [input_obj] * D
                dims = [d for d in range(D)]

                for tidx, t in enumerate(times):
                    self.HAL.set_input_rates(objs, dims, rates[tidx, :], time=t, flush=False)

        start_sweep(get_raw_spikes, get_outputs) # this will cause a flush
        enqueue_input_vals(input_vals) # no flush yet
        self.HAL.flush()

        sleeptime = (end_time - start_time) / 1e9
        time.sleep(sleeptime + TFUDGE * 2)

        outputs, spikes = end_sweep(get_raw_spikes, get_outputs, start_time, end_time)

        return outputs, spikes

