"""Find the synapse's pulse extender speed

Derived from check_max_synapse_rates.py

For each synapse:
Set up the Tag Action Table to send +1 and -1 spikes to an individual synapse for each input spike
generated from one of the FPGA's spike generators.
Send a high rate to the synapse, well above its maximum possible input consumption rate
The input -> fifo -> tat -> synapse path will quickly back up
and, due to the datapath operation,  the rate of overflows will match the synapse's consumption
rate
"""
import os
from time import sleep
from time import time as get_time
import argparse
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
np.set_printoptions(precision=2)

import pystorm
from pystorm.hal.calibrator import Calibration
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.exp import clear_overflows, compute_spike_gen_rates
from utils.file_io import set_data_dir

CORE = 0
NRN_N = 4096
DIM = 1

SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAT_STOP_BIT = 1
TAT_SIGN_0 = 0
TAT_SIGN_1 = 1

FIFO_BUFFER_SIZE = 255

DEFAULT_SYN_PU = pystorm.hal.DAC_DEFAULTS['DAC_SYN_PU'] # analog bias setting
DEFAULT_SYN_PD = pystorm.hal.DAC_DEFAULTS['DAC_SYN_PD'] # analog bias setting

class SynPulseExtenderCalibration(Calibration):
    """Calibrates the synapse pulse extender circuit

    Parameters
    ----------
    syn_n: int
        number of synapses to test
    test_time: float
        time per synapse to collect overflow data
    slop_time: float
        time to allow traffic to flush at the start and end of an experiment
    """
    def __init__(self, syn_n,
                 syn_pd=DEFAULT_SYN_PD, syn_pu=DEFAULT_SYN_PU,
                 input_rate=20000, spike_gen_time_unit_ns=10000,
                 test_time=1.0, slop_time=0.2):
        subdir = "dac_syn_pd_" + str(syn_pd)
        super(SynPulseExtenderCalibration, self).__init__(__file__, subdir=subdir)
        self.syn_pd = syn_pd
        self.syn_pu = syn_pu
        self.syn_n = syn_n
        self.spike_gen_rate = compute_spike_gen_rates(
            input_rate-1, input_rate, spike_gen_time_unit_ns)[0]
        self.test_time = test_time
        self.slop_time = slop_time

    def build_net(self, hal):
        """Builds the HAL-level network for testing"""
        tap_matrix = np.zeros((NRN_N, DIM))
        net = graph.Network("net")
        net.create_pool("p", tap_matrix)
        hal.map(net)

    def setup_analog(self, hal):
        """Sets the synapse config bits and the bias currents"""
        hal.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD, self.syn_pd)
        hal.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU, self.syn_pu)
        for n_idx in range(NRN_N):
            hal.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.DISABLED)
        for s_idx in range(self.syn_n):
            hal.driver.SetSynapseEnableStatus(CORE, s_idx, bd.bdpars.SynapseStatusId.ENABLED)
        hal.flush()

    def setup_hal(hal):
        """Set the HAL traffic settings"""
        # clear queues
        hal.set_time_resolution(downstream_ns=self.spike_gen_time_unit_ns, upstream_ns=10000000)
        hal.start_traffic(flush=False)
        hal.disable_spike_recording(flush=False)
        hal.disable_output_recording(flush=True)

    def setup_tat(self, hal, syn_idx):
        """Set up the tag action table to send spikes to an individual synapse"""
        hal.stop_traffic(flush=True)
        sleep(self.slop_time)
        hal.start_traffic(flush=True)
        addr = HAL.driver.BDPars.GetSynAERAddr(syn_idx)
        tat_entry = bd.PackWord([
            (bd.TATSpikeWord.STOP, TAT_STOP_BIT),
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_0, addr),
            (bd.TATSpikeWord.SYNAPSE_SIGN_0, TAT_SIGN_0),
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_1, addr),
            (bd.TATSpikeWord.SYNAPSE_SIGN_1, TAT_SIGN_1)])
        hal.driver.SetMem(CORE, bd.bdpars.BDMemId.TAT0, [tat_entry], TAT_START_ADDR)
        hal.flush()

    def toggle_spk_generator(self, hal, rate, test_time, slop_time):
        """Toggle the spike generator and check for overflow"""
        clear_overflows(HAL, slop_time)
        test_time_ns = int(test_time*1E9)
        slop_time_ns = int(slop_time*1E9)
        cur_time_ns = HAL.get_time()
        HAL.driver.SetSpikeGeneratorRates(
            CORE, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=cur_time_ns+slop_time_ns)
        HAL.driver.SetSpikeGeneratorRates(
            CORE, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=cur_time_ns+test_time_ns+slop_time_ns)
        sleep(test_time + 2*slop_time)
        overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
        return overflow_0

    def test_syn(self, syn_idx):
        """Deliver spikes to a synapse to find its spike consumption rate"""
        set_tat(syn_idx)
        # check overflow rate at max spike gen rate to predict max synapse rate
        overflows = toggle_spk_generator(SPIKE_GEN_RATE, self.test_time, self.slop_time)
        overflow_rate = overflows / self.test_time
        max_rate = 2 * overflow_rate # 2 spikes per TAT entry
        print("Synapse {}, Input Rate {:.1f} overflow_count {} overflow rate {:.1f}".format(
            syn_idx, SPIKE_GEN_RATE, overflows, overflow_rate))
        return max_rate

    def run(self, hal):
        self.max_rates = np.zeros(self.syn_n)

        self.build_net(hal)
        self.setup_analog(hal)
        self.setup_hal(hal)
        start_time = get_time()
        for syn_idx in range(SYN_N):
            self.max_rates[syn_idx] = self.test_syn(syn_idx)
            self.report_time_remaining(start_time, syn_idx)

    def report_time_remaining(self, start_time, syn_idx):
        """Occasionally estimate and report the remaining time"""
        if syn_idx%4 == 0 and self.syn_n > 1:
            n_syn_completed = syn_idx+1
            delta_time = get_time()-start_time
            est_time_remaining = delta_time/(syn_idx+1) * (self.syn_n-n_syn_completed)
            print("\tEstimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
                est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))

    def save_data(self, hal):
        np.savetxt(self.data_dir + "max_rates.txt", self.max_rates)
        pulse_widths = (1./max_rates).reshape((32, 32))
        hal.add_calibration("synapse", "pulse_width_dac_{}".format(self.syn_pd), pulse_widths)

    def plot_data(self):
        """Plot the data"""
        max_rates = self.max_rates
        syn_n = len(max_rates)
        max_rates_mean = np.mean(max_rates)
        max_rates_min = np.min(max_rates)
        max_rates_max = np.max(max_rates)

        fig_1d, axs = plt.subplots(figsize=(8, 6))
        axs.plot(max_rates, 'o', markersize=1.5)
        axs.set_xlabel("Synapse Index")
        axs.set_ylabel("Max Input Rate (spks/s)")
        fig_1d.savefig(self.fig_dir + "syn_idx_vs_max_rate.pdf")

        if syn_n > 1: # make histograms
            fig_hist, axs = plt.subplots(ncols=2, figsize=(12, 6))

            axs[0].hist(max_rates, bins=80)
            axs[0].axvline(max_rates_mean, color="k", linewidth=1, label="mean")
            axs[0].set_xlabel("Max Input Rate (spks/s)")
            axs[0].set_ylabel("Counts")
            axs[0].set_title(
                "Full Rate Histogram\n"+
                "Min:{:,.0f} Mean:{:,.0f} Max:{:,.0f}".format(
                    max_rates_min, max_rates_mean, max_rates_max))
            axs[1].plot(np.sort(max_rates), (np.arange(syn_n)+1)/syn_n)
            axs[1].axvline(max_rates_mean, color="k", linewidth=1, label="mean")
            axs[1].set_xlabel("Max Input Rate (spks/s)")
            axs[1].set_ylabel("Cumulative Probability")
            axs[1].set_title("Full Rate Cumulative Distribution Function")

            fig_hist.suptitle("All Synapses")
            fig_hist.savefig(self.fig_dir + "histogram.pdf")

        if syn_n == NRN_N//4: # all syn_n tested
            sqrt_n = int(np.ceil(np.sqrt(syn_n)))
            max_rates_2d = max_rates.reshape((sqrt_n, -1))
            fig_heatmap, axs = plt.subplots()
            ims = axs.imshow(max_rates_2d)
            plt.colorbar(ims)
            axs.set_xlabel("Synapse X Coordinate")
            axs.set_ylabel("Synapse Y Coordinate")
            axs.set_title("Max Input Rate (spks/s)")
            fig_heatmap.savefig(self.fig_dir + "2d_heatmap.pdf")

            max_rates_hex = np.nan*np.ones((sqrt_n, sqrt_n*2+1))
            max_rates_hex[0::2, 0:sqrt_n*2:2] = max_rates_2d[0::2, :]
            max_rates_hex[0::2, 1:sqrt_n*2:2] = max_rates_2d[0::2, :]
            max_rates_hex[1::2, 1:sqrt_n*2:2] = max_rates_2d[1::2, :]
            max_rates_hex[1::2, 2:sqrt_n*2+1:2] = max_rates_2d[1::2, :]
            fig_hex_heatmap, axs = plt.subplots()
            matplotlib.cm.get_cmap().set_bad(color='w')
            ims = axs.imshow(max_rates_hex, aspect=2)
            axs.set_xticks([])
            axs.set_yticks([])
            plt.colorbar(ims)
            axs.set_title("Max Input Rate (spks/s)")
            fig_hex_heatmap.savefig(self.fig_dir + "2d_hex_heatmap.pdf")

            tile_max_rates = dict(
                upper_left=max_rates_2d[0::2, 0::2].flatten(),
                upper_right=max_rates_2d[0::2, 1::2].flatten(),
                lower_left=max_rates_2d[1::2, 0::2].flatten(),
                lower_right=max_rates_2d[1::2, 1::2].flatten())
            fig_tile, axs = plt.subplots(ncols=3, figsize=(20, 6))
            offset = 0
            for pos in tile_max_rates:
                hist_values, bin_edges = np.histogram(tile_max_rates[pos], bins=50)
                bin_centers = bin_edges[:-1] + np.diff(bin_edges)/2
                offset -= np.max(hist_values)
                axs[0].fill_between(
                    bin_centers, np.ones_like(hist_values)*offset, hist_values+offset, label=pos)
            axs[0].set_xlabel("Max Input Rate (spks/s)")
            axs[0].set_yticks([])
            axs[0].set_title("Histograms")
            axs[0].legend()
            cdf_idxs = {}
            for pos in tile_max_rates:
                n_syn = len(tile_max_rates[pos])
                cdf_values = (np.arange(n_syn)+1) / n_syn
                cdf_idxs[pos] = np.sort(tile_max_rates[pos])
                axs[1].plot(cdf_idxs[pos], cdf_values, alpha=1.0, label=pos)
            axs[1].set_xlabel("Max Input Rate (spks/s)")
            axs[1].set_ylabel("Probability")
            axs[1].set_title("Cumulative Distribution Functions")
            axs[1].legend()
            positions = list(tile_max_rates.keys())
            for idx0, pos0 in enumerate(positions):
                for pos1 in positions[idx0+1:]:
                    axs[2].plot(cdf_idxs[pos0], cdf_idxs[pos1],
                                linewidth=1, label="{} : {}".format(pos0, pos1))
            xlim = axs[2].get_xlim()
            ylim = axs[2].get_ylim()
            min_val = np.min(xlim + ylim)
            max_val = np.max(xlim + ylim)
            axs[2].plot([min_val, max_val], [min_val, max_val], 'k', linewidth=1)
            axs[2].set_xlim(xlim)
            axs[2].set_ylim(ylim)
            axs[2].set_xlabel("Position 0's Max Input Rate (spks/s)")
            axs[2].set_ylabel("Position 1's Max Input Rate (spks/s)")
            axs[2].set_title("Quantile-Quantiles")
            axs[2].legend(title="Position 0 : Position 1")
            fig_tile.suptitle("Dividing Synapses by Position in Tile")
            fig_tile.savefig(self.fig_dir + "syn_tile.pdf")

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the synapse pulse extender')
    parser.add_argument("--syn_n", dest="syn_n", type=int, default=1024,
                        help="number of synapses to calibrate (Default 1024)")
    parser.add_argument("--syn_pd", dest="syn_pd", type=int, default=DEFAULT_SYN_PD,
                        help="Set DAC_SYN_PD bias (Default {})".format(DEFAULT_SYN_PD))
    args = parser.parse_args()
    return args

def calibrate_syn_pulse_extender(parsed_args):
    """Run the calibration"""
    syn_pd = parsed_args.syn_pd
    syn_n = parsed_args.syn_n
    calibration = SynPulseExtenderCalibration(syn_n, syn_pd)
    hal = HAL()
    calibration.run(hal)
    calibration.save_data(hal)
    calibration.plot_data(hal)

if __name__ == "__main__":
    calibrate_syn_pulse_extender(parse_args())
