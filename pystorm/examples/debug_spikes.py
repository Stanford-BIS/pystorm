import time
import pickle

from pystorm.PyDriver import bddriver as bd

CORE = 0
downstream_time_res = 10 * 1000 # ns
upstream_time_res = downstream_time_res * 10 # ns

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
else:
    pass

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

time.sleep(2)

driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_EXC    , 512)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC     , 544)  # Alex says 900 required for balancing
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_INH    , 512)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_LK     , 10)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD     , 10)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU     , 1024)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_G     , 1024)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_R     , 1)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 1)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF   , 1)

for addr in range(256):
    driver.OpenDiffusorAllCuts(CORE, addr)

for addr in range(1024):
    driver.DisableSynapse(CORE, addr)

for addr in range(4096):
    driver.DisableSoma(CORE, addr)
    driver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    driver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)

yaddr = [32]
xaddr = [32]

faddr = []

spk_data = dict()

for y0 in yaddr:
    for x0 in xaddr:
        addr = driver.GetSomaAERAddr(x0, y0)
        faddr.append(y0 * 64 + x0)
        driver.EnableSoma(CORE, addr)
        driver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE_FOURTH)
        driver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.POSITIVE)
        driver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)

driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 1)

driver.Flush()

driver.RecvXYSpikes(CORE)
val = 10
print("[INFO] Setting refractory DAC to %d" % val)
driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF, val)

for addr in faddr:
    spk_data[addr] = []
_invalid = []

time.sleep(10)
_ = driver.RecvXYSpikes(CORE)
time.sleep(30)
_qdebug = driver.GetOutputQueueCounts()
_addr, _times = driver.RecvXYSpikes(CORE)
print("[INFO] Got %d spikes" % len(_addr))
print(_qdebug)

for _a, _t in zip(_addr, _times):
    if _a in faddr:
        spk_data[_a].append(_t)
    else:
        _invalid.append(_a)

if len(_invalid) > 0:
    print("[WARNING] Received %d invalid addresses" % len(_invalid))
    print(_invalid)

OFILE = open("spk_data_%d.bin" % val, "wb")
pickle.dump(spk_data, OFILE)
OFILE.close()

from numpy import diff, sort, median
from matplotlib.pyplot import figure, semilogy, savefig, subplot, hist, xlabel, tight_layout, title, ioff, show

ioff()

def plot_hist(value, bin1=100, bin2=25, prefix='', idx=2145):
    IFILE = open("%sspk_data_%d.bin" % (prefix, value), "rb")
    _data = pickle.load(IFILE)
    _times = _data[idx]
    _isi = diff(_times)
    _isi_ms = _isi * 1e3
    _freq = 1./_isi
    _freq_khz = _freq / 1000

    figure()
    semilogy(sort(_isi_ms), '.')
    savefig("spk_isi_sorted_%d.pdf" % value)

    figure()
    subplot(211)
    hist(_isi_ms, bin1, histtype='stepfilled')
    xlabel("ISI (ms)")
    title("Median = %.3e ms" % median(_isi_ms))
    subplot(212)
    hist(_freq_khz, bin2, histtype='stepfilled')
    xlabel("Freq (KHz)")
    title("Median = %.3e KHz" % median(_freq_khz))
    tight_layout()
    savefig("spk_dist_%d.pdf" % value)

    show()

plot_hist(val, idx=faddr[0])
