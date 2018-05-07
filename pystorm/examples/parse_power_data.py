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

def data_sample_stats(raw_data, sample_len=1000):
    """ Calculate the stats of sample distribution of the mean"""
    mean = np.mean(raw_data)
    #var = np.var(raw_data)
    #print("Variance: {}".format(var))
    std_dev = np.std(raw_data)
    N = len(raw_data)
    means = []
    std_devs = []
    for i in range(int(np.floor(N/sample_len))):
        cur_trial = raw_data[i*sample_len:(i+1)*sample_len]
        cur_mean = np.mean(cur_trial)
        means.append(cur_mean)
        cur_std_dev = np.std(cur_trial)/np.sqrt(sample_len)
        std_devs.append(cur_std_dev)

    #std_dev_mean = std_dev/np.sqrt(N)
    std_dev_mean = np.mean(std_devs)
    
    #print("Means: {} \n{}".format(mean, means))
    #print("Std Devs: {} {}\n{}".format(std_dev, std_dev_mean, std_devs))
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


def PlotMovingAvg(data, timestamps, win_len=100):
    # plot line of the moving average, using a window length of win_len
    plt.plot(moving_average(timestamps, win_len),
            moving_average(data, win_len),
            '-', alpha=1)

def moving_average(a, n=10):
    ret = np.cumsum(a, dtype=float)
    ret[n:] = ret[n:] - ret[:-n]
    return ret[n - 1:] / n

def PlotPM(curPMs, bin_count=100, win_len=None, plot_stat_lines=False):
    powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])

    u_mean, sigma_mean, SNR_mean = data_sample_stats(powers)
    print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
        % (curPMs[1],u_mean, sigma_mean, SNR_mean))

    ax1 = plt.subplot(121)
    PlotPowerVsTime(powers, curTimestamps, curPMs[1])
    if win_len != None:
        PlotMovingAvg(powers, curTimestamps, win_len)
    ax2 = plt.subplot(122)
    PlotPowerHist(powers, bin_count, curPMs[1], plot_stat_lines)

    return ax1, ax2

def PlotPowerEachFullRange(PM_list, bin_count=100, save_path='', lbl_prefix=''):
    for j, curPMs in enumerate(PM_list):
        plt.figure(num="Full"+str(j),figsize=(14,7))
        curPlotLabel = lbl_prefix+"Run"+curPMs[1]
        PlotPM(curPMs, win_len=100, plot_stat_lines=True)

        if save_path!='':
            plt.savefig(save_path+"/PowerConsumptionPlots_%s.png" % curPlotLabel)


def PlotPowerFullRange(PM_list, bin_count=100, save_path='', lbl_prefix=''):
    plt.figure(num=1,figsize=(14,7))
    for j, curPMs in enumerate(PM_list):
        PlotPM(curPMs, bin_count)

    if save_path!='':
        plt.savefig(save_path+"/PowerConsumptionPlots%s.png" % lbl_prefix)


def PlotPowerSubRanges(PM_list, windowLen, vertical_lines=[], bin_count=100, save_path='', lbl_prefix=''):
    """Plots the a sub-range of the full Power timeline
    PM_list: list of power measurements
    windowLen: every "windowLen" readings, create a new sub-range plot
    vertical_lines: list of points where vertical lines should be marked
    bin_count: number of bins for histogram
    save_path: path to which figures should be saved
    lbl_prefix: prefix for labelling purposes
    """
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

            u_mean, sigma_mean, SNR_mean = data_sample_stats(power_sub_sample)
            print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
                % (curPlotLabel, u_mean, sigma_mean, SNR_mean))

            vertical_lines, _, _ = get_switch_times(curTimestamps,
                    n_cycles=40, duration=20, skip_durations=3)

            ax1 = plt.subplot(221)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample, curPlotLabel)
            for wl in range(int(windowLen/80), int(windowLen/20),int(windowLen/100)):
                PlotMovingAvg(power_sub_sample, timestamps_sub_sample, wl)
            [plt.axvline(x=_line, color='gray', linestyle='--', linewidth=0.5, alpha=0.5) for _line in vertical_lines[(vertical_lines > min(timestamps_sub_sample)) &                     (vertical_lines < max(timestamps_sub_sample))]]
            ax2 = plt.subplot(222)
            PlotPowerHist(power_sub_sample, bin_count, plot_stat_lines=True)
            ax3 = plt.subplot(223)
            PlotPowerVsTime(powers, curTimestamps, curPlotLabel)
            PlotPowerVsTime(power_sub_sample, timestamps_sub_sample)
            PlotMovingAvg(powers, curTimestamps, int(windowLen/10))
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


def PlotVM(curVMs, dc_offset=0.0, bin_count=100, win_len=None, plot_stat_lines=False):
    curIndices, curVoltages, curTimestamps = getVoltVals(curVMs[0], dc_offset)

    u_mean, sigma_mean, SNR_mean = data_sample_stats(curVoltages)
    print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
        % (curVMs[1], u_mean, sigma_mean, SNR_mean))

    ax1 = plt.subplot(121)
    PlotVoltVsTime(curVoltages, curTimestamps, curVMs[1])
    if win_len != None:
        PlotMovingAvg(curVoltages, curTimestamps, win_len)
    ax2 = plt.subplot(122)
    PlotVoltHist(curVoltages, bin_count, curVMs[1], plot_stat_lines)

    return ax1, ax2

def get_switch_times(timestamps, n_cycles, duration, skip_durations):
    """ This function takes a list of timestamps and returns the times within the timestamp range at which the test should have switched, based on the duration setting of the test"""
    start_time = skip_durations * duration
    end_time = start_time + 2*n_cycles*duration + 1
    switch_times = np.array(range(start_time, end_time, duration))
    #print("Switching times: {}".format(np.array(switch_times)))

    return switch_times, start_time, end_time

def get_powers_stats(powers, timestamps, n_cycles, duration, skip_durations=3):
    """ 
    Returns the means and std devs of powers of interleaved power states
    This test assumes you are switching between two power modes every "duration" seconds
    The means and std devs are returned for each interleaved power state, as such, the return variables are lists of length 2*n_cycles
    
    n_cycles: number of times the system switched between power modes (low and high)
    duration: the time between which the test switches between the two power states
    skip_durations: number of duration increments to ignore at the beginning of the test
    """
    switch_times, start_time, end_time = get_switch_times(timestamps, n_cycles, duration, skip_durations)
#    for t_low, t_hi in zip(switch_times, switch_times[1:]):
#        print(t_low, t_hi)
    
    powers_means = []
    powers_vars = []
    len_powers = []
    len_times = []
    fudge_time = 1  # ignore any data within fudge_time seconds of a switching time
    if n_cycles > end_time:
        print("Error: Too many cycles specified, compared to data set. Ask for fewer cycles")
    else:
        for i in range(2*n_cycles):
#            print(timestamps[timestamps>switch_times[i]])
            mask = (timestamps > switch_times[i]+fudge_time) & (timestamps < switch_times[i+1]-fudge_time)
            curPowers = powers[mask]
            curTimes = timestamps[mask]
            len_powers.append(len(curPowers))
            len_times.append(len(curTimes))
            powers_means.append(np.mean(curPowers))
            powers_vars.append(np.var(curPowers))
            #print(curPowers)
            #print(curTimes)

        print("Avg. number of samples: {}".format(
            np.mean(len_powers)))
        num_samples = np.floor(np.mean(len_powers))
    #print(powers_means)
    return powers_means, powers_vars, num_samples

def GetAvgDifferenceOfMeans(PM_list, n_cycles, duration,
        skip_cycles=3, bin_count=100, num_ops=1,
        save_path='', lbl_prefix=''):
    means = []
    for j, curPMs in enumerate(PM_list):
        curLabel = save_path[j]+lbl_prefix[j]+"Run"+curPMs[1]
        print("Test Run:=========\n{}".format(curLabel))

        powers, curIndices, curVoltages, curTimestamps = getPowerVals(curPMs[0])

#        u_mean, sigma_mean, SNR_mean = data_sample_stats(powers, 1000)
#        print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
#            % (curPMs[1],u_mean, sigma_mean, SNR_mean))
#        means.append(u_mean)

        powers_interleaved_means, powers_interleaved_vars, num_samples = get_powers_stats(powers, curTimestamps, n_cycles[j], duration[j], skip_cycles[j])
        #print("List of mean of powers: {}".format(np.array(powers_interleaved_means)))
        #print("List of var of powers: {}".format(np.array(powers_interleaved_vars)))
        
        lo_powers = np.array(powers_interleaved_means[::2])
        #print("List of low powers: {}".format(lo_powers))
        hi_powers = np.array(powers_interleaved_means[1::2])
        #print("List of high powers: {}".format(hi_powers))
        lo_mean_power = np.mean(lo_powers)
        hi_mean_power = np.mean(hi_powers)
        print("Mean power values\n \tlow power: {} mW\n".format(lo_mean_power) + 
                "\thigh power: {} mW".format(hi_mean_power))

        print("Average power consumption of the operation: ")
        print("\t{} W".format(hi_mean_power*10**-3 - lo_mean_power*10**-3))
        print("Average energy consumption of the operation: ")
        print("\t{} J/op".format((hi_mean_power*10**-3 - lo_mean_power*10**-3)/num_ops[j]))

        #print("Power Means:\n{}".format(np.array(powers_interleaved_means)))

        # Get the difference between the mean powers of each sample
        diff_of_means = np.array([np.abs(x1 - x2) for x1, x2 in zip(powers_interleaved_means, powers_interleaved_means[1:])])
        num_diff = len(diff_of_means)
        #print("Diff of Powers Means:\n{}".format(diff_of_means))
        print("Number of difference pairs: {}".format(num_diff))
            
        # Calculuate the mean difference of mean powers
        mean_diff_of_mean_powers = np.mean(diff_of_means)
        print("\nMean of Diff of Mean Powers: {}".format(mean_diff_of_mean_powers))

        #lo_var_power = np.var(lo_powers)
        #hi_var_power = np.var(hi_powers)
        #lohi_cov_power = np.cov(lo_powers, hi_powers)[0][1]
        #print("Var_lo: {}\t Var_hi: {}".format(lo_var_power, hi_var_power))
        #print("Cov: {}".format(lohi_cov_power))
        #var_diff_of_mean_powers = hi_var_power + lo_var_power - 2*lohi_cov_power

        # Calculate Var[E[u_iH-u_iL]] as 1/N^2*1/K*Sum(Var(u_iH)+Var(u_iL))
        sum_of_var = np.sum(powers_interleaved_vars)
        var_mean_diff_of_mean_powers = (1/num_diff**2)*(1/num_samples)*sum_of_var 
        print("Var of Mean Diff of Mean Powers: {}".format(var_mean_diff_of_mean_powers))
        print("Std Dev of Mean Diff of Mean Powers: {}".format(np.sqrt(var_mean_diff_of_mean_powers)))
        print("SNR: {}".format(mean_diff_of_mean_powers/np.sqrt(var_mean_diff_of_mean_powers)))

        # sample half the samples from the diff of means list
        bootstrap_means = []
        for i in range(100):
            bootstrap_means.append(np.mean(np.random.choice(diff_of_means, int(num_diff/2), replace=False)))
            #if i%10 == 0:
            #    print(np.var(bootstrap_means))

        bootstrap_mean = np.mean(bootstrap_means)
        bootstrap_var = np.var(bootstrap_means)
        #print("Bootstrap Mean: {}\tBootstrap Var: {}".format(bootstrap_mean, bootstrap_var))
        bootstrapped_SNR = mean_diff_of_mean_powers/np.sqrt(bootstrap_var)
        print("Bootstrap SNR: {}\n".format(bootstrapped_SNR))

def ComparePvsV(PM_list, VM_list, dc_offset=0.0, bin_count=100, save_path='', lbl_prefix=''):
    if len(PM_list)==len(VM_list):
        for j in range(len(PM_list)):
            curPMs = PM_list[j]
            curVMs = VM_list[j]
            curPlotLabel = lbl_prefix+"Run"+curPMs[1]
            powers, P_Indices, P_Voltages, P_Timestamps = getPowerVals(curPMs[0])
            V_Indices, V_Voltages, V_Timestamps = getVoltVals(curVMs[0], dc_offset)

            plt.figure(num="Compare_Full_"+str(j),figsize=(16,8))

            u_mean_p, sigma_mean_p, SNR_mean_p = data_sample_stats(powers)
            print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
                % (curPMs[1], u_mean_p, sigma_mean_p, SNR_mean_p))

            u_mean_v, sigma_mean_v, SNR_mean_v = data_sample_stats(V_Voltages)
            print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
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

            u_mean_p1, sigma_mean_p1, SNR_mean_p1 = data_sample_stats(powers1)
            print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
                % (curPMs1[1], u_mean_p1, sigma_mean_p1, SNR_mean_p1))

            u_mean_p2, sigma_mean_p2, SNR_mean_p2 = data_sample_stats(powers2)
            print("[%s] Mean:\t%f\tAvg. Std Err of the Mean:\t%f\tSNR:\t%f"
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

index_array = [
##        "000",
#        "001",
##        "002",
##        "003",
#        "004",
##        "005",
#        "006",
##        "007",
#        "008",
##        "009",
        "010",
##        "011",
##        "012",
##        "013",
##        "014",
##        "015",
##        "016",
##        "017",
##        "018",
##        "019",
        ]
test_type = [
        "FIFO_InputRate5MHz",
        "TapPoint_AERRX_InputRate8kHzW64H64",
#        "Static",
        "Decode_SomaBias875_dVal0.0026_Dout3",
        "AERTX_SomaBias875_dVal0.0078125",
        "InputIO_InputRate5MHz",
        "AERRX_InputRate5MHz",
        ]
#test_type = "InputIO_InputRate5MHz"
#test_type = "Static_to_InputIO_InputRate5MHz"
#test_type = "Static"

test_type = [
#        "FIFO_InputRate5MHz_hiaccuracy_6kreadings",
#        "TapPoint_AERRX_InputRate8kHzW64H64_hiaccuracy_20kreadings",
#        "Decode_SomaBias875_dVal0.0026_Dout3_hiaccuracy_2.5kreadings",
#        "AERTX_SomaBias875_dVal0.0078125_hiaccuracy_9kreadings",
        "InputIO_InputRate5MHz_hiaccuracy_9kreadings",
#        "AERRX_InputRate8kHzW64H64_hiaccuracy_20kreadings",
        ]

#test_type = "AERTX_SomaBias875_dVal0.0078125"

M1_list = []
M2_list = []

file_path = [
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/InputIO_FIFO_Vdd1.006",
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/FIFO_TAT-AERRX_Vdd1.006", 
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/AERTX_PAT+ACC_Vdd1.006", 
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/KillNeurons_AERTX_Vdd1.006", 
        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/Static_InputIO_Vdd1.006", 
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/Static_AERRX_Vdd1.006", 
        ]
sv_path = [
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/InputIO_FIFO_Vdd1.006",
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/FIFO_TAT-AERRX_Vdd1.006",
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/AERTX_PAT+ACC_Vdd1.006", 
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/KillNeurons_AERTX_Vdd1.006", 
        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/Static_InputIO_Vdd1.006", 
#        "PowerMeasurements_BDTB2_Green1_22C_Paper_InterleavedPowerTests/Static_AERRX_Vdd1.006", 
        ]

for i, idx in enumerate(index_array):
    if isinstance(file_path, str):
        L1Measurements = getData(file_path+"/smua1buffer%s.csv" % (idx))
        M1_list.append((L1Measurements,idx))
        L2Measurements = getData(file_path+"/smub1buffer%s.csv" % (idx))
        M2_list.append((L2Measurements,idx))
    elif isinstance(file_path, list):
        L1Measurements = getData(file_path[i]+"/smua1buffer%s.csv" % (idx))
        M1_list.append((L1Measurements,idx))
        L2Measurements = getData(file_path[i]+"/smub1buffer%s.csv" % (idx))
        M2_list.append((L2Measurements,idx))

#for i, runNum in enumerate(index_array):
#    PlotPowerEachFullRange([M1_list[i]], save_path=sv_path, lbl_prefix=test_type[i]+"_1V0D_")
#    #PlotPowerSubRanges([M1_list[i]], 1000, save_path=sv_path, lbl_prefix=test_type[i]+"_1V0D_")


#PlotPowerFullRange(M1_list, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
PlotPowerEachFullRange(M1_list, save_path=sv_path[0], lbl_prefix=test_type[0]+"_1V0D_")
#ComparePvsV(M1_list, M2_list, dc_offset=1.0, save_path=sv_path)
#ComparePvsP(M2_list, M1_list, dc_offset=0.0, save_path=sv_path, lbl_prefix=test_type+"_1V0Avs1V0D_")
PlotPowerSubRanges(M1_list, 1000, save_path=sv_path[0], lbl_prefix=test_type[0]+"_1V0D_")
#PlotPowerSubRanges(M2_list, 10000, save_path=sv_path, lbl_prefix=test_type+"_1V0A_")
#plt.show()

# For all three power tests
#lbl_prefix_list = ["_"+tt+"_1V0D" for tt in test_type]
#GetAvgDifferenceOfMeans(M1_list, [34, 43, 20, 40], [15, 40, 10, 20], skip_cycles=[3, 3, 3, 3], num_ops=[5e6, 8e3*1024, 65594880, 26915146], save_path=sv_path, lbl_prefix=lbl_prefix_list)
# For InputIO_FIFO
#GetAvgDifferenceOfMeans(M1_list, 34, 15, skip_cycles=3, num_ops=5e6, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
# For FIFO_TAT-AERRX
#GetAvgDifferenceOfMeans(M1_list, 43, 40, skip_cycles=3, num_ops=8e3*1024, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
# For AERTX_PAT+ACC
#GetAvgDifferenceOfMeans(M1_list, 20, 10, skip_cycles=3, num_ops=65594880, save_path=sv_path, lbl_prefix="_"+test_type+"_1V0D")
# For KillNeurons_AERTX
#GetAvgDifferenceOfMeans(M1_list, [40], [20], skip_cycles=[3], num_ops=[26915146], save_path=[sv_path], lbl_prefix="_"+test_type+"_1V0D")
# For Static_InputIO
GetAvgDifferenceOfMeans(M1_list, [40], [20], skip_cycles=[3], num_ops=[5000000], save_path=sv_path, lbl_prefix="_"+test_type[0]+"_1V0D")
# For Static_AERRX
#GetAvgDifferenceOfMeans(M1_list, [40], [20], skip_cycles=[3], num_ops=[5000000], save_path=[sv_path], lbl_prefix="_"+test_type+"_1V0D")
