import pickle
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.net_builder import NetBuilder
from pystorm.hal.run_control import RunControl
from pystorm.hal.calibrator import Calibrator, PoolSpec

# create a big pool
# plot the spikes

# Pool parameters
X = 10
Y = 10
NNEURON = Y*X
DISABLE_THRESHOLD = 0.8 # disable somas spiking above this percentage

def run_spikes_test(net, run_control):
    """Run a single input test trial"""
    test_time = 1
    test_time_ns = int(test_time*1E9)

    input_rates = np.zeros((2, 1))
    input_times = np.arange(2)*test_time_ns
    input_vals = {net.input:(input_times, input_rates)}

    _, spike_data = run_control.run_input_sweep(
        input_vals, get_raw_spikes=True, get_outputs=False)
    spikes, bin_times_ns = spike_data
    spikes = spikes[net.pool]

    bin_times = bin_times_ns * 1E-9
    spike_rates = np.sum(spikes, axis=0) / (bin_times[-1] - bin_times[0])
    return spike_rates

def plot_spike_data(spike_rates):
    """Plot the spike rates"""
    spike_rates = spike_rates.reshape((Y, X))
    min_nz_rate = np.min(spike_rates[spike_rates > 0])

    fig = plt.figure(figsize=(14, 6))
    axs = [plt.subplot2grid((2, 3), (0, ax_idx)) for ax_idx in range(3)]
    axs += [plt.subplot2grid((2, 3), (1, 0), colspan=3)]
    axs[0].imshow(spike_rates)
    axs[1].imshow(np.log(spike_rates + min_nz_rate/10))
    axs[2].imshow(spike_rates > 0)
    axs[0].set_title("linear scale")
    axs[1].set_title("log scale (0s clipped)")
    axs[2].set_title("zero/nonzero rates")

    axs[3].plot(spike_rates.flatten(), 'o')
    axs[3].set_xlabel("neuron index")
    return fig, axs

def build_pool_spec():
    TPM = np.zeros((Y, X))
    TPM[::2, ::2] = 1
    if np.sum(TPM)%2 == 1:
        y_idx, x_idx = np.nonzero(TPM)
        TPM[y_idx[-1], x_idx[-1]] = 0
    TPM = TPM.reshape((-1, 1))
    ps = PoolSpec(
        label="pool",
        YX=(10, 10), loc_yx=(0, 0),
        D=1,
        biases=3, gain_divisors=1,
        TPM=TPM,
    )
    return ps

def test_soma_disable():
    """Runs the test"""
    ps = build_pool_spec()
    hal = HAL()
    net_builder = NetBuilder(hal)
    calibrator = Calibrator(hal)

    net = net_builder.create_single_pool_net_from_spec(ps)
    run_control = RunControl(hal, net)
    hal.map(net)

    spike_rates = run_spikes_test(net, run_control)
    fig, _ = plot_spike_data(spike_rates)
    fig.suptitle("before disabling somas")

    ########### disable somas ###########
    threshold = np.sort(spike_rates)[int(DISABLE_THRESHOLD*X*Y)]
    y_idxs, x_idxs = np.nonzero(spike_rates.reshape((Y, X)) > threshold)
    for y_idx, x_idx in zip(y_idxs, x_idxs):
        hal.driver.DisableSomaXY(0, x_idx, y_idx)
    hal.flush()
    ########### disable somas ###########

    spike_rates = run_spikes_test(net, run_control)
    fig, _ = plot_spike_data(spike_rates)
    fig.suptitle("after disabling somas spiking above {:.0f} spks/s".format(threshold))

    plt.show()

if __name__ == "__main__":
    test_soma_disable()
