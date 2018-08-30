from .graph_object import GraphObject, FanoutError
import pystorm.hal.neuromorph.hardware_resources as hwr

class Output(GraphObject):
    def __init__(self, label, dimensions):
        super(Output, self).__init__(label)
        self.dimensions = dimensions

    def __repr__(self):
        return "Output " + self.label + ". dims = " + str(self.dimensions)

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("Sink", hwr.Sink(self.dimensions))

    def create_connection_resources(self):
        if (len(self.out_conns) > 0):
            raise FanoutError(self, max_fanout=0)

    def _connect_from(self, src, src_resource_key, conn):
        self._check_conn_from_type(src, ["Bucket"])
        # add TATFanout to prevent clobbering
        src_resource = src._get_resource(src_resource_key)

        TAT_fanout = hwr.TATFanout(src.get_num_dimensions()) # create TAT fanout
        sink = self._get_resource("Sink")

        # make connections
        src_resource.connect(TAT_fanout)
        TAT_fanout.connect(sink)

        # append to resources
        src._append_resource(("TATFanout", self), TAT_fanout)

    @property
    def filter_idxs(self):
        return self._get_resource("Sink").filter_idxs
