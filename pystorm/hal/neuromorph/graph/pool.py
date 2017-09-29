class Pool(object):
    def __init__(self, label, n_neurons, dimensions):
        self.label = label
        self.n_neurons = n_neurons
        self.dimensions = dimensions

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons
