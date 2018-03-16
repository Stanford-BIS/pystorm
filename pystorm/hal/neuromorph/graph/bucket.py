from .graph_object import GraphObject
import pystorm.hal.neuromorph.hardware_resources as hwr

class Bucket(GraphObject):
    def __init__(self, label, dimensions):
        super(Bucket, self).__init__(label)
        self.label = label
        self.dimensions = dimensions

    def __repr__(self):
        return "Bucket " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("AMBuckets", hwr.AMBuckets(self.dimensions))

    def create_connection_resources(self):
        if len(self.out_conns) == 1:
            conn, tgt = self._get_single_conn_out()
            tgt._connect_from(self, "AMBuckets", conn)

        elif len(self.conns_out) > 1:
            # need to make a TATFanout object to initiate conns from

            bucket = self._get_resource("AMBuckets")
            TAT_fanout = hwr.TATFanout(self.dimensions) # create TAT fanout entries

            bucket.connect(TAT_fanout)

            # standard (str, None) key in resources for TATFanout, doesn't belong to any one connection
            self._append_resource("TATFanout", TAT_fanout) 

            for conn in self.out_conns:
                tgt = conn.dest
                tgt._connect_from(self, "TATFanout", conn)

    def _connect_from(self, src, src_resource_key, conn):
        self._check_conn_from_type(src, ["Input", "Bucket", "Pool"])
        src_resource = src._get_resource(src_resource_key)

        # anything but the neuron array (which has the PAT) needs a TAT acc entry
        if not isinstance(src_resource, hwr.Neurons):
            TAT_acc = hwr.TATAccumulator(src.get_num_dimensions()) # create TAT acc entries
        weights = hwr.MMWeights(conn.weights) # create weights
        bucket = self._get_resource("AMBuckets")

        # make connections
        if not isinstance(src_resource, hwr.Neurons):
            src_resource.connect(TAT_acc)
            TAT_acc.connect(weights)
        else:
            src_resource.connect(weights)
        weights.connect(bucket)

        # append to resources
        if not isinstance(src_resource, hwr.Neurons):
            src._append_resource(("TATAccumulator", self), TAT_acc)
        src._append_resource(("MMWeights", self), weights)


