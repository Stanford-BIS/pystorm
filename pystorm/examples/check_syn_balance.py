import time
import pickle
from numpy import diff, sort, median, array, zeros
import numpy as np

from pystorm.PyDriver import bddriver as bd

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
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 1)

CORE = 0

for addr in range(256):
    driver.OpenDiffusorAllCuts(CORE, addr)

for addr in range(1024):
    driver.DisableSynapse(CORE, addr)

for addr in range(4096):
    driver.DisableSoma(CORE, addr)
    driver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    driver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.THREE)

time_ns = 0
r = 10  # Hz  (0 stops the rate generator)
tat_start_addr = 0  # each address is a pair of TAGs
# index into each address in memory (i.e., for each pair), not TAG address
tat_index = 0
gen_index = 0  # from 256

faddr = []
baseline_data = dict()
spk_data = []

xs = [16]
ys = [16]

rates = list(range(0, 1001, 100))
op_rate = zeros(len(rates) + 1, dtype=np.float)


# sign 1 => exc, sign 0 => inh
sign0 = 1
sign1 = 0

tat_entries = []

for y0 in ys:
    for x0 in xs:
        addr = driver.GetSynAERAddr(x0, y0)
        driver.EnableSynapse(CORE, addr)

        stop = 0
        if y0 == ys[-1] and x0 == xs[-1]:
            stop = 1
        tat_entry = bd.PackWord([
            (bd.TATSpikeWord.STOP              , stop)  ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_0 , addr) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_0    , sign0) ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_1 , addr) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_1    , sign1) ])
        tat_entries.append(tat_entry)

        soma_addr = driver.GetSomaAERAddr(x0 * 2, y0 * 2)
        driver.EnableSoma(CORE, soma_addr)
        faddr.append(y0 * 64 * 2 + x0 * 2)

driver.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, tat_start_addr)

def set_rate(gen_idx, rate):
    driver.SetSpikeGeneratorRates(CORE, [gen_idx], [tat_index], [rate], time_ns, False)
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
    time.sleep(5)
    _ = driver.RecvXYSpikes(CORE)
    _t0 = driver.GetFPGATimeSec()
    time.sleep(2)
    _qdebug = driver.GetOutputQueueCounts()
    _addr, _times_ns = driver.RecvXYSpikes(CORE)
    _t1 = driver.GetFPGATimeSec()
    print("[INFO] Received: %d spikes in %g s" % (len(_addr), _t1 - _t0))
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
    set_rate(gen_index, _r)
    time.sleep(5)
    run_exp(spk_data[_idx])

driver.Stop()

from matplotlib.pyplot import savefig, figure, subplot, hist, xlabel, ylabel, tight_layout, ion, show, suptitle, plot, sca, gca
from matplotlib.ticker import EngFormatter

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

    suptitle("Median ISI: %.3gms" % median(_isi_ms))

    tight_layout()
    show()
    return(median(_isi))

figure()
op_rate[0] = plot_summary(baseline_data)
savefig("baseline_spk.pdf")

for _idx, _r in enumerate(rates):
    figure()
    op_rate[_idx + 1] = plot_summary(spk_data[_idx])
    savefig("ip_rate_%d.pdf" % _r)

formatter = EngFormatter(unit='Hz')

figure()
subplot(211)
plot(rates, 1./op_rate[1:], '.-')
xlabel(r"$f_\mathrm{in}$ (Hz)")
ylabel(r"$f_\mathrm{out}$ (Hz)")
gca().xaxis.set_major_formatter(formatter)
gca().yaxis.set_major_formatter(formatter)

subplot(212)
plot(rates, 1./op_rate[1:] - 1./op_rate[0], '.-')
xlabel(r"$f_\mathrm{in}$ (Hz)")
ylabel(r"$f_\mathrm{out} - f_\mathrm{out, 0}$ (Hz)")
gca().xaxis.set_major_formatter(formatter)
gca().yaxis.set_major_formatter(formatter)

tight_layout()
savefig("fi_fo_summary.pdf")

input("Press any key...")
