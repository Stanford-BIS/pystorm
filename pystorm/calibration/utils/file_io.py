"""Utility functions for file IO in calibration modules"""
import os
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

def setup_save_dir(dirname):
    """Setup a directory for saving data or figures"""
    if not os.path.isdir(dirname):
        os.makedirs(dirname, exist_ok=True)

def set_data_dir(fname, subdir=""):
    """Sets up a data directory for a given filename"""
    data_dir = "./data/" + os.path.basename(fname)[:-3] + "/"
    if subdir:
        data_dir += "/" + subdir
    if not os.path.isdir(data_dir):
        os.makedirs(data_dir, exist_ok=True)
    return data_dir
