class Pool(object):
    """Represents a pool of neurons
    
    Parameters
    ----------
    label: string
        name of pool
    encoders:
        encoder matrix (pre-diffuser), size neurons-by-dimensions.
        Elements must be in {-1, 0, 1}.
        Implicitly describes pool dimensionality and number of neurons.
    x: int
        neuron pool is physically a rectangle; x dimension of neuron pool
    y: int
        neuron pool is physically a rectangle; y dimension of neuron pool
    """
    def __init__(self, label, encoders, x, y):
        self.label = label
        self.encoders = encoders
        self.n_neurons, self.dimensions = encoders.shape
        self.x = x
        self.y = y
        assert(self.n_neurons == x * y)

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons
