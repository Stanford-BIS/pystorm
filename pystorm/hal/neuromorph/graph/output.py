class Output(object):
    def __init__(self, label, dimensions):
        self.label = label
        self.dimensions = dimensions

    def __gt__(self, output2):
        return self.label > output2.label

    def __repr__(self):
        return "Output " + self.label

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions
