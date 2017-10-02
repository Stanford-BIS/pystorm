class Bucket(object):
    def __init__(self, label, dimensions):
        self.label = label
        self.dimensions = dimensions

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions
