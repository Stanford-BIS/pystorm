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
            PowerMeasurements.append(PowerMeasurement(float(row[0]), float(row[1]), float(row[5])))
    return PowerMeasurements

class PowerMeasurement(object):
    def __init__(self):
        self.index = 0
        self.voltage = 0
        self.timestamp = 0
    def __init__(self, iVal, vVal, tVal):
        self.index = iVal
        self.voltage = vVal
        self.timestamp = tVal

index_array = ["00","01","02","03_0","03_1","03_2","04","05","06",]
index_array = ["06",]
PM_list = []
for i in index_array:
    PowerMeasurements = getData("FullPipelinePowerTests/smua1buffer0%s.csv" % i)
    PM_list.append(PowerMeasurements)

#PowerMeasurements = getData("FullPipelinePowerTests/smua1buffer005.csv")
#PM_list = []
#PM_list.append(PowerMeasurements)

print()

for j, curPMs in enumerate(PM_list):
    curVoltages = []
    curTimestamps = []
    for i, curPM in enumerate(curPMs):
        if i == 0:
            base_timestamp = curPM.timestamp
        #print(curPM.voltage)
        curVoltages.append(curPM.voltage)
        curTimestamps.append(curPM.timestamp-base_timestamp)

    curVoltages = [v * 1000 for v in curVoltages]  # convert curVoltages to mW

    r_val = 1.   # resistor value in ohms
    vdd = 1.0   # voltage rail being measure in volts

    powers = [v/r_val * vdd for v in curVoltages]
#print(powers)
    print("Mean: %f" % np.mean(powers))

    plt.figure(1)
    plt.plot(curTimestamps, powers)

    plt.figure(2)
    plt.hist(powers, bins=100)

plt.figure(1)
plt.xlabel("t (s)")
plt.ylabel("P (mW)")
plt.title("Power Consumption over time")
#plt.savefig("PowerComsumption_vs_Time.png")

plt.figure(2)
plt.xlabel("P (mW)")
plt.ylabel("Count")
plt.title("Power Consumption Histogram")
#plt.savefig("PowerComsumption_Histogram.png")

plt.show()

