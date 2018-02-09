"""Utility functions for calibration modules"""
import sys
import numpy as np

def load_txt_data(fname, dtype=None):
    """Load text data from fname"""
    try:
        if dtype:
            data = np.loadtxt(fname, dtype=dtype)
        else:
            data = np.loadtxt(fname)
    except FileNotFoundError:
        print("\nError: Could not find saved data {}\n".format(fname))
        sys.exit(1)
    return data

def load_npy_data(fname, dtype=None):
    """Load npy data from fname"""
    try:
        data = np.load(fname)
    except FileNotFoundError:
        print("\nError: Could not find saved data {}\n".format(fname))
        sys.exit(1)
    return data
