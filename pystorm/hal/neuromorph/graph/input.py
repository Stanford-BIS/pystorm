class Input(object):
    def __init__(self, label, dimensions):
        self.label = label
        self.dimensions = dimensions

    def __gt__(self, input2):
        return self.label > input2.label

    def __repr__(self):
        return "Input " + self.label

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions
