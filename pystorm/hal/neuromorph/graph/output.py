from graph_object import GraphObject, ConnectionTypeError, FanoutError
import pystorm.hal.neuromorph.hardware_resources as hwr

class Output(GraphObject):
    def __init__(self, label, dimensions):
        super(GraphObject, self).__init__(label)
        self.dimensions = dimensions

    def __repr__(self):
        return "Output " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("Sink", hwr.Sink(self.dimensions))

    def create_connection_resources(self):
        raise FanoutError(self)

    def get_filter_idxs(self):
        return self._get_resource("Sink").filter_idxs
