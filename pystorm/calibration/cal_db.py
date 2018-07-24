import numpy as np
import pandas as pd
import sys

class CalibrationDB(object):
    
    def __init__(self, fname_base):
        """
        Basically a container for a dict of pandas dataframes.

        We're enforcing a particular access pattern and structure
        """
        self.fname_base = fname_base

        self.SYNAPSE_CALS = [
            'tau',
            'pulse_width',
            'high_bias_magnitude',
            ]

        self.SOMA_CALS = [
            ]

        self.CAL_TYPES = {'synapse': self.SYNAPSE_CALS, 
                          'soma': self.SOMA_CALS}

        # TODO, should probably inherit these parameters somehow
        # if the list gets too long, think about it
        self.SOMA_YX = 64
        self.SYNAPSE_YX = 32

        self.CAL_SIZES = {'synapse': (self.SYNAPSE_YX, self.SYNAPSE_YX), 
                          'soma': (self.SOMA_YX, self.SOMA_YX)}

        # enforce structure of constants
        for k in self.CAL_SIZES:
            assert(k in self.CAL_TYPES)
            assert(isinstance(self.CAL_SIZES[k], tuple) and len(self.CAL_SIZES[k]) == 2)
            assert(isinstance(self.CAL_TYPES[k], list))

        # CalibrationDB includes a mechanism to determine which chip is which
        # special-purpose DF to store chip activation patterns
        # in this case, the chips are columns, instead of being indices
        self.activations = pd.DataFrame()
        self.ACTIVATION_MATCH_THR = .7

        # main data storage objects

        # dict of DFs keyed by cal_objs, e.g. "soma", "synapse"
        # self.cals[cal_obj] is a pandas DF with multilevel indexing indexed [chip, y, x]
        # to get a single data element: self.cals[cal_obj][cal_type][chip][obj_idx]
        #   NOTE, the above works (maybe isn't fast) for viewing data, 
        #   but to assign data correctly, you need to do: 
        #   self.cals[cal_obj].loc[(chip, yidx, xidx), cal_type] = blah
        #     also note that slicing in .loc is tricky, see below
        # examples:
        # if I wanted all the soma bias calibrations for 'chipA' I would do:
        # self.cals['soma']['bias']['chipA']
        # if I wanted that calibration for the neuron at (y,x) = 7,5:
        # self.cals['soma']['bias']['chipA'][7][5]
        # if I wanted that calibration for the neuron at yx index 40,
        # self.cals['soma']['bias']['chipA'].iloc[40]
        # or soma_bias neurons (y,x) = (0:8, 0:8) for all chips
        # self.cals['soma']['bias'].loc['chipA', 0:7, 0:7]
        #   NOTE to assign, use slice() (DF must still be sorted)
        #   self.cals['soma'].loc['chipA', slice(0,7), slice(0,7)), 'bias'] = blah
        # etc.
        self.clear_dataframes()

        for cal_obj in self.CAL_TYPES:
            self.load_from_file(cal_obj)

    def clear_dataframes(self):
        self.cals = {}
        for cal_obj in self.CAL_TYPES:
            self.cals[cal_obj] = pd.DataFrame()

    def get_obj_fname(self, cal_obj):
        return self.fname_base + '_' + cal_obj + '.csv'

    def load_from_file(self, cal_obj):
        fname = self.get_obj_fname(cal_obj)
        try:
            df = pd.DataFrame.from_csv(fname)
            self.cals[cal_obj] = df 
        except FileNotFoundError:
            pass # this is OK, we'll just create them later
        except:
            print("Unexpected error when trying to load CalibrationDB from file:",
                  sys.exc_info()[0])
            raise

    def write_to_file(self, cal_obj):
        fname = self.get_obj_fname(cal_obj)
        try:
            self.cals[cal_obj].to_csv(fname)
        except:
            print("Unexpected error when trying to write CalibrationDB to file:",
                  sys.exc_info()[0])
            raise

    def make_cyx_index(self, chip, y, x):
        mg = np.meshgrid(range(y), range(x))
        col_arrays = [mg[0].flatten(), mg[1].flatten()]
        cs = [chip] * len(col_arrays[0])
        col_tuples = list(zip(cs, *col_arrays))
        index = pd.MultiIndex.from_tuples(col_tuples, names=['chip_name', 'y', 'x'])
        return index

    def make_cal_obj_index(self, chip, cal_obj):
        ydim, xdim = self.CAL_SIZES[cal_obj]
        return self.make_cyx_index(chip, ydim, xdim)

    def get_activation_similarities(self, activation):
        sims = []
        for chip in self.activations:
            chip_activation = self.activations[chip]
            sims.append(np.dot(chip_activation, activation))
        return sims
        
    def find_chip(self, activation):
        """Given an activation, determine whether activation matches any previously-seen chip"""
        sims = self.get_activation_similarities(activation)

        match_sims = sims > np.array(self.ACTIVATION_MATCH_THR)
        num_match = np.sum(match_sims)
        match_chips = []
        for match_sim, chip in zip(match_sims, self.activations.columns):
            if match_sim:
                match_chips.append(chip)

        if len(match_chips) == 0:
            return None
        if len(match_chips) == 1:
            return match_chips[0]
        else:
            errstr = " ".join(["activation matched >1 following chips", match_chips, "exiting"])
            raise ValueError(errstr)

    def add_new_chip(self, activation, chip):
        """Add a new chip to the database"""
        # ensure that the chip you're trying to add doesn't exist
        # if there's one match, do nothing
        matching_chip = self.find_chip(activation)

        if matching_chip is None:
            new_activations = pd.Series(activation, 
                    index=self.make_cal_obj_index(None, 'soma'))
            self.activations[chip] = new_activations
            for cal_obj in self.CAL_TYPES:
                empty_rows = pd.DataFrame(index=self.make_cal_obj_index(chip, cal_obj))
                self.cals[cal_obj] = self.cals[cal_obj].append(empty_rows)
        else:
            raise ValueError('adding chip that already exists!')

    def check_cal_pars(self, chip, cal_obj, cal_type):
        if cal_obj not in self.cals:
            errstr = " ".join(["check_cal_pars(): you supplied calibration object", cal_obj,
                  "which isn't known to CalibrationDB.",
                  "All calibrations must be registered in self.cals",
                  "supported objects:", str(self.cals.keys())])
            raise ValueError(errstr)

        if cal_type not in self.CAL_TYPES[cal_obj]:
            errstr = " ".join(["check_cal_pars(): you supplied calibration type", cal_type, 
                  "which isn't known to CalibrationDB.",
                  "All calibrations must be registered in self.CAL_TYPES",
                  "supported calibration types for this object:", str(self.CAL_TYPES[cal_obj])])
            raise ValueError(errstr)

        if chip not in self.activations.columns:
            errstr = " ".join(["check_cal_pars(): you supplied chip", chip, 
                  "which is not in the CalibrationDB, use add_new_chip() first, exiting"])
            raise ValueError(errstr)

    def add_calibration(self, chip, cal_obj, cal_type, values, value_indices=None, commit_now=True):
        """
        Add new data to soma calibration for a particular chip

        Inputs:
        =======
        chip: Chip name
        cal_obj: Name of thing being calibrated (e.g "soma" or "synapse")
        cal_type: Calibration name
        values: Set of values to enter into calibration DB
                if value_indices is None, must have as many entries as there are objects
                (e.g. 4096 or 1024 for somas or synapses)
        indices: Set of indices to use for less-than-full length entries. 
                 Can be y,x or flat index, can be out-of-order.
        """
        self.check_cal_pars(chip, cal_obj, cal_type)

        if value_indices is None:
            if isinstance(values, np.ndarray):
                flat_values = values.flatten()

            dimy, dimx = self.CAL_SIZES[cal_obj]
            cal_size = dimy * dimx
            if len(flat_values) != cal_size:
                errstr = " ".join(["add_calibration(): values for", cal_obj, "must have", 
                        str(cal_size), "entries, exiting"])
                raise ValueError(errstr)

            else:
                #if cal_type not in self.cals[cal_obj].columns
                #    new_df = pd.DataFrame(flat_values, 
                #            index=self.make_cal_obj_index(chip, cal_obj),
                #            columns=[cal_type])
                #    self.cals[cal_obj] = new_df
                #else:
                self.cals[cal_obj].sort_index(inplace=True)
                self.cals[cal_obj].loc[(chip, slice(0,dimy), slice(0,dimx)), cal_type] = flat_values
        else:
            raise ValueError("value_indices != None is not yet supported")

        if commit_now:
            self.write_to_file(cal_obj)
            

    def get_calibration(self, chip, cal_obj, cal_type):
        """Get a copy of the calibration DF for a particular calibration

        Inputs:
        =======
        chip: Chip name
        cal_obj: Name of thing being calibrated (e.g "soma" or "synapse")
        cal_type: Calibration name

        Returns:
        =======
        Pandas dataframe indexed y,x with the requested data
        """

        self.check_cal_pars(chip, cal_obj, cal_type)

        self.cals[cal_obj].sort_index(inplace=True)
        return self.cals[cal_obj].loc[(chip, slice(None), slice(None)), cal_type]

