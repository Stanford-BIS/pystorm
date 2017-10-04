import numpy as np

class Pool(object):
    def __init__(self, label, n_neurons, dimensions,
                 taps=None, tap_signs=None):
        self.label = label
        self.n_neurons = n_neurons
        self.dimensions = dimensions

        if taps is not None:
            assert tap_signs is not None
            taps = np.array(taps)
            tap_signs = np.array(tap_signs)
            assert taps.shape == tap_signs.shape
            assert len(taps.shape) == 2
            K, D = taps.shape
            assert D == dimensions
            assert (K % 2) == 0
            assert (taps < self.n_neurons).all()
        self.taps = taps
        self.tap_signs = tap_signs

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons

    def get_taps(self):
        return self.taps

    def get_tap_signs(self):
        return self.tap_signs
