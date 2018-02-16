class Bucket(object):
    def __init__(self, label, dimensions):
        self.label = label
        self.dimensions = dimensions

    def __gt__(self, bucket2):
        return self.label > bucket2.label

    def __repr__(self):
        return "Bucket " + self.label

    def get_label(self):
        return self.label

    def get_num_dimensions(self):
        return self.dimensions
