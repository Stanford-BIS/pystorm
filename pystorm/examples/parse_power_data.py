import numpy as np
import matplotlib.pyplot as plt
import time
import csv
import glob

from os import listdir, walk
from os.path import isfile, join

# This program parses power measurement data gotten from the Keithley 2604B
#   SourceMeter.

def getData(csvFileName, numHeaderRows=6):
    Measurements = []
    with open(csvFileName, newline='') as csvfile:
        csvReader = csv.reader(csvfile)
        for headerRow in range(numHeaderRows):
            next(csvReader)
        for row in csvReader:
            #print("Voltage: %.9f, Timestamp: %.9f" % (float(row[1]), float(row[5])))
            Measurements.append([int(row[0]), float(row[1]), float(row[5])])
    return np.array(Measurements)

# getPowerVals gets converts the power measurements from voltages to power values
#   PowerMeasurements: 2-D array of indices, voltages, and temperatures, over time
#   r_val: resistor value in ohms
#   vdd: voltage rail being measure in volts
def getPowerVals(PowerMeasurements, r_val=1., vdd=1.0):
        indices = PowerMeasurements[:,0]
        voltages = PowerMeasurements[:,1]
        timestamps = PowerMeasurements[:,2] - PowerMeasurements[0,2]

        voltages = voltages * 1000    # convert voltages to mW

        powers = voltages/r_val * vdd
        #print(powers)
        return (powers, indices, voltages, timestamps)

# getVoltVals gets converts the power measurements from voltages to power values
#   VoltMeasurements: 2-D array of indices, voltages, and temperatures, over time
#   r_val: resistor value in ohms
#   vdd: voltage rail being measure in volts
def getVoltVals(VoltMeasurements, dc_offset=0.0):
        indices = VoltMeasurements[:,0]
        voltages = VoltMeasurements[:,1]
        timestamps = VoltMeasurements[:,2] - VoltMeasurements[0,2]

        voltages = (voltages-dc_offset) * 1000 # remove dc_offset and convert voltages to mW

        return (indices, voltages, timestamps)

def PlotPowerVsTime(powers, timestamps, runVal=None):
    plt.plot(timestamps, powers, '.-', alpha=0.5)
    plt.xlabel("t (s)")
    plt.ylabel("P (mW)")
    if runVal == None:
        plt.title("Power consumption over time")
    else:
        plt.title("Power consumption over time - Run: %s" % runVal)

def PlotPowerHist(powers, bin_count=100, runVal=None, plot_stat_lines=False):
    plt.hist(powers, bins=bin_count, alpha=0.4)
    if plot_stat_lines:
        plt.axvline(x=np.mean(powers), color='b')
        plt.axvline(x=np.mean(powers)+np.std(powers), color='c')
        plt.axvline(x=np.mean(powers)-np.std(powers), color='c')
    plt.xlabel("P (mW)")
    plt.ylabel("Count")
    if runVal == None:
        plt.title("Power consumption histogram")
    else:
        plt.title("Power consumption histogram - Run: %s" % runVal)


def PlotPM(curPMs, bin_count=100, plot_stat_lines=False):
    powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])
    print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
        % (curPMs[1],np.mean(powers),np.std(powers),np.mean(powers)/np.std(powers)))

    ax1 = plt.subplot(121)
    PlotPowerVsTime(powers, curTimestamps, curPMs[1])
    ax2 = plt.subplot(122)
    PlotPowerHist(powers, bin_count, curPMs[1], plot_stat_lines)

    return ax1, ax2

def PlotPowerEachFullRange(PM_list, bin_count=100, save_path='', lbl_prefix=''):
    for j, curPMs in enumerate(PM_list):
        plt.figure(num="Full"+str(j),figsize=(14,7))
        curPlotLabel = lbl_prefix+"Run"+curPMs[1]
        PlotPM(curPMs, plot_stat_lines=True)

        if save_path!='':
            plt.savefig(save_path+"/PowerConsumptionPlots_%s.png" % curPlotLabel)


def PlotPowerFullRange(PM_list, bin_count=100, save_path='', lbl_prefix=''):
    plt.figure(num=1,figsize=(14,7))
    for j, curPMs in enumerate(PM_list):
        PlotPM(curPMs, bin_count)

    if save_path!='':
        plt.savefig(save_path+"/PowerConsumptionPlots%s.png" % lbl_prefix)


def PlotPowerSubRanges(PM_list, windowLen, bin_count=100, save_path='', lbl_prefix=''):
    for j, curPMs in enumerate(PM_list):
        powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])

        numSegments = int(np.ceil(len(powers)/windowLen))
        for i in range(numSegments):
            curPlotLabel = lbl_prefix+"Run"+curPMs[1]+"_"+str(i+1)+"of"+str(numSegments)
            plt.figure(num=curPlotLabel,figsize=(12,8))
            #print(powers[i*windowLen:(i+1)*windowLen])
            power_sub_sample = powers[i*windowLen:(i+1)*windowLen]
            timestamps_sub_sample = curTimestamps[i*windowLen:(i+1)*windowLen]
            norm_timestamps_sub_sample = curTimestamps[i*windowLen:(i+1)*windowLen] - curTimestamps[i*windowLen]
            print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
                    % (curPlotLabel, np.mean(power_sub_sample),np.std(power_sub_sample), np.mean(power_sub_sample)/np.std(power_sub_sample)))

            ax1 = plt.subplot(221)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample, curPlotLabel)
            ax2 = plt.subplot(222)
            PlotPowerHist(power_sub_sample, bin_count, plot_stat_lines=True)
            ax3 = plt.subplot(223)
            PlotPowerVsTime(powers, curTimestamps, curPlotLabel)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample)
            ax4 = plt.subplot(224)
            PlotPowerHist(powers, bin_count)
            PlotPowerHist(power_sub_sample, bin_count)

            if save_path!='':
                plt.savefig(save_path+"/PowerConsumptionPlots_%s.png" % (curPlotLabel))


def PlotVoltVsTime(voltages, timestamps, runVal=None):
    plt.plot(timestamps, voltages, '.-', alpha=0.5)
    plt.xlabel("t (s)")
    plt.ylabel("V (mV)")
    if runVal == None:
        plt.title("Voltage over time")
    else:
        plt.title("Voltage over time - Run: %s" % runVal)


def PlotVoltHist(voltages, bin_count=100, runVal=None, plot_stat_lines=False):
    plt.hist(voltages, bins=bin_count, alpha=0.4)
    if plot_stat_lines:
        plt.axvline(x=np.mean(voltages), color='b')
        plt.axvline(x=np.mean(voltages)+np.std(voltages), color='c')
        plt.axvline(x=np.mean(voltages)-np.std(voltages), color='c')
    plt.xlabel("V (mV)")
    plt.ylabel("Count")
    if runVal == None:
        plt.title("Voltage histogram")
    else:
        plt.title("Voltage histogram - Run: %s" % runVal)


def PlotVM(curVMs, dc_offset=0.0, bin_count=100, plot_stat_lines=False):
    curIndices, curVoltages, curTimestamps = getVoltVals(curVMs[0], dc_offset)
    print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
        % (curVMs[1],np.mean(curVoltages),np.std(curVoltages),np.mean(curVoltages)/np.std(curVoltages)))

    ax1 = plt.subplot(121)
    PlotVoltVsTime(curVoltages, curTimestamps, curVMs[1])
    ax2 = plt.subplot(122)
    PlotVoltHist(curVoltages, bin_count, curVMs[1], plot_stat_lines)

    return ax1, ax2

def ComparePvsV(PM_list, VM_list, dc_offset=0.0, bin_count=100, save_path='', lbl_prefix=''):
    if len(PM_list)==len(VM_list):
        for j in range(len(PM_list)):
            curPMs = PM_list[j]
            curVMs = VM_list[j]
            curPlotLabel = lbl_prefix+"Run"+curPMs[1]
            powers, P_Indices, P_Voltages, P_Timestamps = getPowerVals(curPMs[0])
            V_Indices, V_Voltages, V_Timestamps = getVoltVals(curVMs[0], dc_offset)

            plt.figure(num="Compare_Full_"+str(j),figsize=(16,8))
            print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
                    % (curPMs[1],np.mean(powers),np.std(powers),np.mean(powers)/np.std(powers)))

            print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
                    % (curVMs[1],np.mean(V_Voltages),np.std(V_Voltages),np.mean(V_Voltages)/np.std(V_Voltages)))

            ax1 = plt.subplot(121)
            plt.plot(P_Timestamps, powers, '.-', alpha=0.5)
            plt.xlabel("t (s)")
            plt.ylabel("P (mW)")

            ax2 = plt.subplot(122)
            plt.hist(powers, bins=bin_count, alpha=0.4)
            plt.axvline(x=np.mean(powers), color='b')
            plt.axvline(x=np.mean(powers)+np.std(powers), color='c')
            plt.axvline(x=np.mean(powers)-np.std(powers), color='c')
            plt.xlabel("P (mW)")
            plt.ylabel("Count")

            ax3 = ax1.twinx()
            ax4 = ax2.twiny()
            ax3.plot(V_Timestamps, V_Voltages, 'r.-', alpha=0.5)
            ax3.set_ylabel("V (mV) (measV-%f V)" % dc_offset)

            ax4.hist(V_Voltages, bins=bin_count, color='r', alpha=0.4)
            ax4.axvline(x=np.mean(V_Voltages), color='r')
            ax4.axvline(x=np.mean(V_Voltages)+np.std(V_Voltages), color='m')
            ax4.axvline(x=np.mean(V_Voltages)-np.std(V_Voltages), color='m')
            ax4.set_xlabel("V (mV) (measV-%f V)" % dc_offset)

            if save_path!='':
                plt.savefig(save_path+"/PowervsVoltage_%s.png" % curPlotLabel)
    else:
        print("ERROR: Power measuerment list & voltage measurement list have different lengths")

def ComparePvsP(PM_list1, PM_list2, dc_offset=0.0, bin_count=100, save_path='', lbl_prefix=''):
    if len(PM_list1)==len(PM_list2):
        for j in range(len(PM_list1)):
            curPMs1 = PM_list1[j]
            curPMs2 = PM_list2[j]
            curPlotLabel = lbl_prefix+"Run"+curPMs1[1]
            powers1, P1_Indices, P1_Voltages, P1_Timestamps = getPowerVals(curPMs1[0])
            powers2, P2_Indices, P2_Voltages, P2_Timestamps = getPowerVals(curPMs2[0])

            plt.figure(num="Compare_Full_"+str(j),figsize=(16,8))
            print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
                % (curPMs1[1],np.mean(powers1),np.std(powers1),np.mean(powers1)/np.std(powers1)))
            print("[%s] Mean:\t%f\tStd Dev:\t%f\tSNR:\t%f"
                % (curPMs2[1],np.mean(powers2),np.std(powers2),np.mean(powers2)/np.std(powers2)))

            ax1 = plt.subplot(121)
            plt.plot(P1_Timestamps, powers1, '.-', alpha=0.5)
            plt.xlabel("t (s)")
            plt.ylabel("P (mW)")

            ax2 = plt.subplot(122)
            plt.hist(powers1, bins=bin_count, alpha=0.4)
            plt.axvline(x=np.mean(powers1), color='b')
            plt.axvline(x=np.mean(powers1)+np.std(powers1), color='c')
            plt.axvline(x=np.mean(powers1)-np.std(powers1), color='c')
            plt.xlabel("P (mW)")
            plt.ylabel("Count")

            ax3 = ax1.twinx()
            ax4 = ax2.twiny()
            ax3.plot(P2_Timestamps, powers2, 'r.-', alpha=0.5)
            ax3.set_ylabel("P (mW)")

            ax4.hist(powers2, bins=bin_count, color='r', alpha=0.4)
            plt.axvline(x=np.mean(powers2), color='r')
            plt.axvline(x=np.mean(powers2)+np.std(powers2), color='m')
            plt.axvline(x=np.mean(powers2)-np.std(powers2), color='m')
            ax4.set_xlabel("P (mW)")

            if save_path!='':
                plt.savefig(save_path+"/PowervsPower_%s.png" % curPlotLabel)
    else:
        print("ERROR: Power measuerment list & voltage measurement list have different lengths")

index_array = ["003"]
test_type = "Static_Long"
M1_list = []
M2_list = []
file_path = "TempControlled_10C_PowerComparisonTests"
sv_path = "TempControlled_10C_PowerComparisonTests"

for i in index_array:
    L1Measurements = getData(file_path+"/smua1buffer%s.csv" % (i))
    M1_list.append((L1Measurements,i))
    L2Measurements = getData(file_path+"/smub1buffer%s.csv" % (i))
    M2_list.append((L2Measurements,i))


#PlotPowerFullRange(M1_list, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
#PlotPowerEachFullRange(M1_list, save_path=sv_path, lbl_prefix=test_type+"_1V0D_")
#ComparePvsV(M1_list, M2_list, dc_offset=1.0, save_path=sv_path)
#ComparePvsP(M1_list, M2_list, dc_offset=0.0, save_path=sv_path, lbl_prefix=test_type+"_1V0Dvs1V0A_")
PlotPowerSubRanges(M1_list, 500, save_path=sv_path, lbl_prefix=test_type+"_1V0D_")
#PlotPowerSubRanges(M2_list, 2000, save_path=sv_path, lbl_prefix=test_type+"_1V0A_")
#plt.show()

