"""Utility functions for file IO in calibration modules"""
import sys
import pickle
import numpy as np

def load_txt_data(fname, dtype=None):
    """Load numpy text data from fname"""
    try:
        if dtype:
            data = np.loadtxt(fname, dtype=dtype)
        else:
            data = np.loadtxt(fname)
    except (FileNotFoundError, IOError) as err:
        print("\nError: {}\n".format(err))
        sys.exit(1)
    return data

def load_npy_data(fname):
    """Load numpy npy data from fname"""
    try:
        data = np.load(fname)
    except (FileNotFoundError, IOError) as err:
        print("\nError: {}\n".format(err))
        sys.exit(1)
    return data

def load_pickle_data(fname):
    """Load pickle data"""
    try:
        with open(fname, "rb") as pickle_file:
            data = pickle.load(pickle_file)
    except (FileNotFoundError, IOError) as err:
        print("\nError: {}\n".format(err))
        sys.exit(1)
    return data

def save_pickle_data(fname, data):
    """Save data to a pickle file"""
    with open(fname, "wb") as pickle_file:
        pickle.dump(data, pickle_file)
