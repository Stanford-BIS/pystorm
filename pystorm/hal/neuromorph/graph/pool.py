class Pool(object):
    """Represents a pool of neurons
    
    Parameters
    ----------
    label: string
        name of pool
    n_neurons: int
        number of neurons in pool
    dimensions: int
        dimensionality of pool
    x: int
        neuron pool is physically a rectangle; x dimension of neuron pool
    y: int
        neuron pool is physically a rectangle; y dimension of neuron pool
    """
    def __init__(self, label, n_neurons, dimensions, x, y):
        self.label = label
        self.n_neurons = n_neurons
        self.dimensions = dimensions
        self.x = x
        self.y = y

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons
