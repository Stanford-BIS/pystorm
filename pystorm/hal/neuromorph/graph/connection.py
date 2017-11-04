class Connection(object):
    def __init__(self, label, src, dest, weights):
        self.label = label
        self.src = src
        self.dest = dest
        self.weights = weights

    def reassign_weights(self, new_weights):
        self.weights = new_weights

    def get_label(self):
        return self.label

    def get_weights(self):
        return self.weights

    def get_source(self):
        return self.src

    def get_dest(self):
        return self.dest
