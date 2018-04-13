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

def data_sample_statistics(raw_data):
    """ Calculate the stats of sample distribution of the mean"""
    mean = np.mean(raw_data)
    std_dev = np.std(raw_data)
    N = len(raw_data)
    std_dev_mean = std_dev/np.sqrt(N)
    SNR_mean = mean/std_dev_mean
    return mean, std_dev_mean, SNR_mean

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

    u_mean, sigma_mean, SNR_mean = data_sample_statistics(powers)
    print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
        % (curPMs[1],u_mean, sigma_mean, SNR_mean))

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

            u_mean, sigma_mean, SNR_mean = data_sample_statistics(power_sub_sample)
            print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
                % (curPlotLabel, u_mean, sigma_mean, SNR_mean))

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

    u_mean, sigma_mean, SNR_mean = data_sample_statistics(curVoltages)
    print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
        % (curVMs[1], u_mean, sigma_mean, SNR_mean))

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

            u_mean_p, sigma_mean_p, SNR_mean_p = data_sample_statistics(powers)
            print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
                % (curPMs[1], u_mean_p, sigma_mean_p, SNR_mean_p))

            u_mean_v, sigma_mean_v, SNR_mean_v = data_sample_statistics(V_Voltages)
            print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
                % (curVMs[1], u_mean_v, sigma_mean_v, SNR_mean_v))

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

            u_mean_p1, sigma_mean_p1, SNR_mean_p1 = data_sample_statistics(powers1)
            print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
                % (curPMs1[1], u_mean_p1, sigma_mean_p1, SNR_mean_p1))

            u_mean_p2, sigma_mean_p2, SNR_mean_p2 = data_sample_statistics(powers2)
            print("[%s] Mean:\t%f\tStd Err of the Mean:\t%f\tSNR:\t%f"
                % (curPMs2[1], u_mean_p2, sigma_mean_p2, SNR_mean_p2))

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

index_array = ["010", "011", "012", "013", "014", "015"]
#index_array = ["000", "001", "002", "007", "008", "009"]
index_array = ["015"]
test_type = [
        "FIFO_InputRate5MHz",
        "TapPoint_AERRX_InputRate7kHzW64H64",
        "Static",
        "Decode_SomaBias875_dVal0.0026_Dout3",
        "AERTX_SomaBias875_dVal0.0078125",
        "InputIO_InputRate5MHz",
        ]
test_type = "InputIO_InputRate5MHz"
#test_type = "FIFO_InputRate5MHz"
#test_type = "TapPoint_AERRX_InputRate7kHzW64H64"
#test_type = "Static"
#test_type = "Decode_SomaBias875_dVal0.0026_Dout3"
#test_type = "AERTX_SomaBias875_dVal0.0078125"

M1_list = []
M2_list = []
file_path = "PowerMeasurements_BDTB2_Green1_22C_Paper_60KReadings"
sv_path = "PowerMeasurements_BDTB2_Green1_22C_Paper_60KReadings"

for i in index_array:
    L1Measurements = getData(file_path+"/smua1buffer%s.csv" % (i))
    M1_list.append((L1Measurements,i))
    L2Measurements = getData(file_path+"/smub1buffer%s.csv" % (i))
    M2_list.append((L2Measurements,i))

#for i, runNum in enumerate(index_array):
#    #PlotPowerEachFullRange([M1_list[i]], save_path=sv_path, lbl_prefix=test_type[i]+"_1V0D_")
#    PlotPowerSubRanges([M1_list[i]], 10000, save_path=sv_path, lbl_prefix=test_type[i]+"_1V0D_")


#PlotPowerFullRange(M1_list, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
#PlotPowerEachFullRange(M1_list, save_path=sv_path, lbl_prefix=test_type+"_1V0D_")
#ComparePvsV(M1_list, M2_list, dc_offset=1.0, save_path=sv_path)
#ComparePvsP(M2_list, M1_list, dc_offset=0.0, save_path=sv_path, lbl_prefix=test_type+"_1V0Avs1V0D_")
PlotPowerSubRanges(M1_list, 10000, save_path=sv_path, lbl_prefix=test_type+"_1V0D_")
#PlotPowerSubRanges(M2_list, 2000, save_path=sv_path, lbl_prefix=test_type+"_1V0A_")
#plt.show()

