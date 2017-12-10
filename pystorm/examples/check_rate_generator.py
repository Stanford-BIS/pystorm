import sys
import os
import time
import pickle
from numpy import diff, sort, median, array, zeros
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
from matplotlib.pyplot import savefig, figure, subplot, hist, xlabel, ylabel, tight_layout, ion, show, suptitle, plot, sca, gca
from matplotlib.ticker import EngFormatter
from pystorm.PyDriver import bddriver as bd

"""
Takes two arguments: Synapse's x and y address
"""

xs = [int(sys.argv[1])]
ys = [int(sys.argv[2])]

# sign 1 => exc, sign 0 => inh
sign0 = 1
sign1 = 1

#rates = list(range(100, 1001, 100))
rates = list(range(0, 1001, 100))
#rates = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
op_rate = zeros(len(rates) + 1, dtype=np.float)

prefix = ("data_%d_%d/" % (xs[0], ys[0]))
try:
    os.mkdir(prefix)
except OSError:
    print("Directory '%s' already exists" % prefix)

CORE = 0
downstream_time_res = 10 * 1000 # ns
upstream_time_res = 1 * downstream_time_res # ns

try:
    driver
except NameError:
    driver = bd.Driver()
    print("[INFO] Starting BD")
    driver.Start() # starts driver threads
    print("[INFO] Resetting BD")
    driver.ResetBD()
    print("[INFO] Initializing DAC")
    driver.InitDAC(CORE)
    print("[INFO] Setting downstream time resolution to [color=ffffff]%d[/color]ns" % downstream_time_res)
    driver.SetTimeUnitLen(downstream_time_res)
    print("[INFO] Setting upstream time resolution to [color=ffffff]%d[/color]ns" % upstream_time_res)
    driver.SetTimePerUpHB(upstream_time_res)
    print("[INFO] Init the FIFO (also turns on traffic)")
    driver.InitFIFO(0)
    print("[INFO] Enable tag traffic")
    driver.SetTagTrafficState(CORE, True, True)
    print("[INFO] Enable spike traffic")
    driver.SetSpikeDumpState(CORE, True, True)
    driver.Flush()
else:
    pass

time.sleep(2)

driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 100)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 25)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 512)

CORE = 0

for addr in range(256):
    driver.OpenDiffusorAllCuts(CORE, addr)

for addr in range(1024):
    driver.DisableSynapse(CORE, addr)

for addr in range(4096):
    driver.DisableSoma(CORE, addr)
    driver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    driver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)

time_ns = 0
tat_start_addr = 0  # each address is a pair of TAGs
# index into each address in memory (i.e., for each pair), not TAG address
tat_index = 0
gen_index = 0  # from 256

faddr = [1048577]
baseline_data = dict()
spk_data = []
approx_rate = []

tat_entries = []

driver.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, tat_start_addr)

def set_rate(gen_idx, rt):
    driver.SetSpikeGeneratorRates(CORE, [gen_idx], [tat_index], [rt], time_ns, False)
    driver.Flush()

driver.Flush()
print('Use [color=ffffff]set_rate(gen_idx, rate)[/color] to adjust the rate')

for _idx in range(len(rates)):
    spk_data.append(dict())

for addr in faddr:
    baseline_data[addr] = []
    for _idx in range(len(rates)):
        spk_data[_idx][addr] = []

def run_exp(data_dict):
    _invalid = []
    time.sleep(1)
    _ = driver.RecvTags(CORE)
    _t0 = driver.GetFPGATime()
    time.sleep(10)
    _qdebug = driver.GetOutputQueueCounts()
    _addr, _times_ns = driver.RecvTags(CORE)
    _t1 = driver.GetFPGATime()
    _num_spikes = len(_addr)
    _int = (_t1 - _t0)
    _rate = _num_spikes / _int * 1e9
    approx_rate.append(_rate)
    print("[INFO] Received: %d spikes in %g s (%g Hz)" % (_num_spikes, _int, _rate))
    print(_qdebug)

    for _a, _t in zip(_addr, _times_ns):
        if _a in faddr:
            data_dict[_a].append(_t)
        else:
            _invalid.append(_a)

    if len(_invalid) > 0:
        print("[WARNING] Received %d invalid addresses" % len(_invalid))
        print(_invalid)

run_exp(baseline_data)

for _idx, _r in enumerate(rates):
    print("[INFO] Setting rate to %d" % _r)
    set_rate(gen_index, 0)
    time.sleep(1)
    set_rate(gen_index, _r)
    time.sleep(1)
    run_exp(spk_data[_idx])

driver.Stop()

ion()

def plot_summary(data_dict):
    _ax1 = subplot(221)
    _ax2 = subplot(223)
    _ax3 = subplot(122)

    for idx in faddr:
        _times_ns = array(data_dict[idx])
        _isi = diff(_times_ns) * 1e-9
        _isi_ms = _isi * 1e3

        sca(_ax1)
        plot(sort(_isi_ms), '.', alpha=0.5)

        sca(_ax2)
        hist(_isi_ms, 25, histtype='stepfilled', alpha=0.5)

        sca(_ax3)
        plot(_times_ns[1:] * 1e-9, _isi_ms, '.', alpha=0.5)

    sca(_ax1)
    xlabel("Spike index")
    ylabel("ISI (ms)")

    sca(_ax2)
    xlabel("ISI (ms)")
    ylabel("Count")

    sca(_ax3)
    xlabel("Time (s)")
    ylabel("ISI (ms)")

    _trange = (_times_ns[-1] - _times_ns[0]) * 1e-9
    _num = len(_times_ns) - 1
    _bin_freq = _num / _trange
    suptitle("Median ISI: %.3gms (%g Hz)" % (median(_isi_ms), _bin_freq))

    tight_layout()
    show()
    return(_bin_freq)

figure()
op_rate[0] = plot_summary(baseline_data)
savefig(prefix + "baseline_spk.pdf")

for _idx, _r in enumerate(rates):
    figure()
    op_rate[_idx + 1] = plot_summary(spk_data[_idx])
    savefig(prefix + ("ip_rate_%d.pdf" % _r))

formatter = EngFormatter(unit='Hz')
approx_rate = array(approx_rate)

print("[INFO] Binned rate")
print(op_rate)
print("[INFO] Approximate rate")
print(approx_rate)

figure()
subplot(211)
plot(rates, op_rate[1:], '.-', label="Spike time based")
plot(rates, approx_rate[1:], '.-', label="FPGA time based")
xlabel(r"$f_\mathrm{in}$ (Hz)")
ylabel(r"$f_\mathrm{out}$ (Hz)")
gca().xaxis.set_major_formatter(formatter)
gca().yaxis.set_major_formatter(formatter)

subplot(212)
plot(rates, op_rate[1:] - op_rate[0], '.-', label="Spike time based")
plot(rates, approx_rate[1:] - approx_rate[0], '.-', label="FPGA time based")
xlabel(r"$f_\mathrm{in}$ (Hz)")
ylabel(r"$f_\mathrm{out} - f_\mathrm{out, 0}$ (Hz)")
gca().xaxis.set_major_formatter(formatter)
gca().yaxis.set_major_formatter(formatter)

tight_layout()
savefig(prefix + ("fi_fo_summary_%d_%d.pdf" % (xs[0], ys[0])))

#input("Press any key...")
