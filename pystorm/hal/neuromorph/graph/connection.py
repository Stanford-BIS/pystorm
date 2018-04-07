class Connection(object):
    def __init__(self, label, src, dest, weights):
        # note, this is a second-class object, not a proper GraphObject (has no resources)
        self.label = label
        self.src = src
        self.dest = dest
        self.weights = weights

        self.src.out_conns.append(self)

    def __repr__(self):
        return "Connection " + self.label

    def __gt__(self, other_obj):
        return self.label > other_obj.label

    def reassign_weights(self, new_weights):
        # note that self.weights = new_weights will not work
        self.weights[:] = new_weights

    def get_weights(self):
        return self.weights

    def get_source(self):
        return self.src

    def get_dest(self):
        return self.dest

    def get_effective_weights(self):
        MMWeights_resource = self.src.resources[("MMWeights", self.dest)]
        return MMWeights_resource.effective_W


