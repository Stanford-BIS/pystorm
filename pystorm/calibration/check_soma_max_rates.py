"""Find the maximum firing rate of the somas as driven by bias

Set soma offset bias to max
Set soma refractory bias to max (minimum refractory)

Iterate through the somas, and collect spikes
Plot results
"""
import os
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D

CORE = 0
MAX_NEURONS = 4096
BIAS_REF = 1024
BIAS_OFFSET = 1024
TIME_SCALE = 1E-9

# NEURONS = 4096
NEURONS = 4
RUN_TIME = .1

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the soma max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args.use_saved_data

def build_net():
    """Builds the HAL-level network for testing"""
    dim = 1
    tap_matrix = np.zeros((MAX_NEURONS, dim))
    net = graph.Network("net")
    pool = net.create_pool("pool", tap_matrix)
    HAL.map(net)
    return pool

def set_analog():
    """Sets the soma config bits and the bias currents"""
    for i in range(4096):
        HAL.driver.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE)
        HAL.driver.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE)
        HAL.driver.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.THREE)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF, BIAS_REF)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, BIAS_OFFSET)
    for n_idx in range(MAX_NEURONS):
        HAL.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.DISABLED)
    HAL.flush()

def toggle_hal_recording():
    """Start and stop HAL traffic"""
    # clear queues
    _ = HAL.get_spikes()
    HAL.set_time_resolution(upstream_ns=10000)
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    sleep(RUN_TIME)
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    HAL.set_time_resolution(upstream_ns=10000000)
    HAL.flush()

def measure_soma_max_rate(pool, nrn_idx):
    """Collect spikes to find a single soma's max firing rate"""
    _ = HAL.get_spikes()
    aer_nrn_idx = HAL.driver.GetSomaAERAddr(nrn_idx)
    HAL.driver.SetSomaEnableStatus(CORE, aer_nrn_idx, bd.bdpars.SomaStatusId.ENABLED)
    toggle_hal_recording()
    HAL.driver.SetSomaEnableStatus(CORE, aer_nrn_idx, bd.bdpars.SomaStatusId.DISABLED)
    hal_spikes = parse_hal_spikes(HAL.get_spikes())
    soma_spikes = np.array(hal_spikes[pool][nrn_idx])[:, 0]
    soma_spikes -= soma_spikes[0]
    soma_spikes = soma_spikes
    n_spks = len(soma_spikes)-1
    time_period = ((soma_spikes[-1]+soma_spikes[-2])/2. - soma_spikes[0])*TIME_SCALE
    max_rate = n_spks/time_period
    return max_rate

def plot_max_rates(max_rates):
    """Plot hte data"""
    neurons = len(max_rates)

    fig_1d = plt.figure()
    plt.plot(max_rates, 'o', markersize=1)
    plt.xlim(0, neurons-1)
    plt.xlabel("Soma Index")
    plt.ylabel("Max Firing Rate (Hz)")

    max_rates_2d = max_rates.reshape((int(np.sqrt(neurons)), -1))
    fig_2d_heatmap = plt.figure()
    ims = plt.imshow(max_rates_2d)
    plt.colorbar(ims)
    plt.xlabel("Soma X Coordinate")
    plt.ylabel("Soma Y Coordinate")
    plt.title("Max Firing Rate")

    fig_2d_surf = plt.figure()
    axs = fig_2d_surf.add_subplot(111, projection='3d')
    xy_idx = np.arange(int(np.sqrt(neurons)))
    x_mesh, y_mesh = np.meshgrid(xy_idx, xy_idx)
    surf = axs.plot_surface(
        x_mesh, y_mesh, max_rates_2d, linewidth=0, cmap=cm.viridis, antialiased=False)
    axs.set_xlabel("Soma X Coordinate")
    axs.set_ylabel("Soma Y Coordinate")
    axs.set_zlabel("Soma Max Firing Rate (Hz)")
    fig_2d_surf.colorbar(surf, shrink=0.5, aspect=5)

    return fig_1d, fig_2d_heatmap, fig_2d_surf

def check_soma_max_rates(use_saved_data):
    """Run the check"""
    if use_saved_data:
        max_rates = np.loadtxt(DATA_DIR + "max_rates.txt")
    else:
        from pystorm.hal import HAL
        from pystorm.hal.hal import parse_hal_spikes
        from pystorm.hal.neuromorph import graph
        from pystorm.PyDriver import bddriver as bd
        pool = build_net()
        set_analog()
        max_rates = np.zeros(NEURONS)
        for nrn_idx in range(NEURONS):
            max_rates[nrn_idx] = measure_soma_max_rate(pool, nrn_idx)

    fig_1d, fig_2d_heatmap, fig_2d_surf = plot_max_rates(max_rates)
    fig_1d.savefig(DATA_DIR + "plot_1d.pdf")
    fig_2d_heatmap.savefig(DATA_DIR + "plot_2d_heatmap.pdf")
    fig_2d_surf.savefig(DATA_DIR + "plot_2d_surface.pdf")
    if not use_saved_data:
        np.savetxt(DATA_DIR + "max_rates.txt", max_rates)
    print("Max firing rates:")
    print(max_rates)
    plt.show()

if __name__ == "__main__":
    use_saved_data = parse_args()
    check_soma_max_rates(use_saved_data)
