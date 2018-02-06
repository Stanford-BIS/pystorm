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
    PowerMeasurements = []
    with open(csvFileName, newline='') as csvfile:
        csvReader = csv.reader(csvfile)
        for headerRow in range(numHeaderRows):
            next(csvReader)
        for row in csvReader:
            #print("Voltage: %.9f, Timestamp: %.9f" % (float(row[1]), float(row[5])))
            PowerMeasurements.append([int(row[0]), float(row[1]), float(row[5])])
    return np.array(PowerMeasurements)

# getPowerVals gets converts the power measurements from voltages to power values
#   PowerMeasurements: 2-D array of indices, voltages, and temperatures, over time
#   r_val: resistor value in ohms
#   vdd: voltage rail being measure in volts
def getPowerVals(PowerMeasurements, r_val=1., vdd=1.0):
        indices = PowerMeasurements[:,0]
        voltages = PowerMeasurements[:,1]
        timestamps = PowerMeasurements[:,2] - PowerMeasurements[0,2]

        voltages = voltages * 1000    # convert curVoltages to mW

        powers = voltages/r_val * vdd
        #print(powers)
        return (powers, indices, voltages, timestamps)


def PlotPowerVsTime(powers, timestamps, runVal=None):
    plt.plot(timestamps, powers, '.-', alpha=0.5)
    plt.xlabel("t (s)")
    plt.ylabel("P (mW)")
    if runVal == None:
        plt.title("Power consumption over time")
    else:
        plt.title("Power consumption over time - Run: %s" % runVal)


def PlotPowerHist(powers, bin_count=100, runVal=None):
    plt.hist(powers, bins=bin_count, alpha=0.4)
    plt.xlabel("P (mW)")
    plt.ylabel("Count")
    if runVal == None:
        plt.title("Power consumption histogram")
    else:
        plt.title("Power consumption histogram - Run: %s" % runVal)


def PlotPM(curPMs, bin_count=100):
    powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])
    print("Mean[%s]: %f" % (curPMs[1],np.mean(powers)))

    plt.subplot(121)
    PlotPowerVsTime(powers, curTimestamps, curPMs[1])
    plt.subplot(122)
    PlotPowerHist(powers, bin_count, curPMs[1])


def PlotEachFullRange(PM_list, bin_count=100, save_path=None):
    for j, curPMs in enumerate(PM_list):
        plt.figure(num="Full"+str(j),figsize=(14,7))
        PlotPM(curPMs)

        if save_path!=None:
            plt.savefig(save_path+"/PowerComsumptionPlots_%s.png" % curPMs[1])


def PlotFullRange(PM_list, bin_count=100, save_path=None):
    plt.figure(num=1,figsize=(14,7))
    for j, curPMs in enumerate(PM_list):
        PlotPM(curPMs, bin_count)

    if save_path!=None:
        plt.savefig(save_path+"/PowerComsumptionPlots.png")


def PlotSubRanges(PM_list, windowLen, bin_count=100, save_path=None):
    for j, curPMs in enumerate(PM_list):
        powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])

        numSegments = int(np.ceil(len(powers)/windowLen))
        for i in range(numSegments):
            curPlotLabel = "Run"+curPMs[1]+"_"+str(i+1)+"of"+str(numSegments)
            plt.figure(num=curPlotLabel,figsize=(12,8))
            #print(powers[i*windowLen:(i+1)*windowLen])
            power_sub_sample = powers[i*windowLen:(i+1)*windowLen]
            timestamps_sub_sample = curTimestamps[i*windowLen:(i+1)*windowLen]
            norm_timestamps_sub_sample = curTimestamps[i*windowLen:(i+1)*windowLen] - curTimestamps[i*windowLen]
            print("Mean[%s]: %f" % (curPlotLabel, np.mean(power_sub_sample)))

            plt.subplot(221)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample, curPlotLabel)
            plt.subplot(222)
            PlotPowerHist(power_sub_sample, bin_count)
            plt.subplot(223)
            PlotPowerVsTime(powers, curTimestamps, curPlotLabel)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample)
            plt.subplot(224)
            PlotPowerHist(powers, bin_count)
            PlotPowerHist(power_sub_sample, bin_count)

            if save_path!=None:
                plt.savefig(save_path+"/PowerComsumptionPlots_%s.png" % (curPlotLabel))

index_array = ["000","001","002","003_0","003_1","003_2","004","005","006",]
index_array = ["006"]#,"003_1",]
PM_list = []

for i in index_array:
    PowerMeasurements = getData("FullPipelinePowerTests/smua1buffer%s.csv" % i)
    PM_list.append((PowerMeasurements,i))


#PlotFullRange(PM_list, save_path=None)
#PlotEachFullRange(PM_list, save_path=None)
#PlotSubRanges(PM_list, 2000, save_path="FullPipelinePowerTests")
#plt.show()

