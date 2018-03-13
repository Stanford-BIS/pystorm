from graph_object import GraphObject, ConnectionTypeError, FanoutError
import pystorm.hal.neuromorph.hardware_resources as hwr

class Bucket(GraphObject):
    def __init__(self, label, dimensions):
        super(GraphObject, self).__init__(label)
        self.label = label
        self.dimensions = dimensions

    def __repr__(self):
        return "Bucket " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("AMBuckets", hwr.AMBuckets(self.dimensions))

    def create_connection_resources(self):
        """
        Conn 4a:
              neuromorph graph: Bucket ─> Bucket
            hardware_resources: AMBuckets ─> MMWeights ─> AMBuckets
        Conn 4b:
              neuromorph graph: Bucket ─> Output
            hardware_resources: AMBuckets ─> TATFanout -> Sink
            (note, AMBuckets -> Sink is technically feasible, but has the repeated outputs problem)
        Conn 4c:
              neuromorph graph: Bucket ─> Pool
            hardware_resources: AMBuckets ─> (TATTapPoint -> Neurons)
         Conn 5:
              neuromorph graph: Bucket ─┬─> Bucket / Output / Pool
                                        └─> Bucket / Output / Pool
                                        ⋮
                                        └─> Bucket / Output / Pool
            hardware_resources: AMBuckets ─> TATFanout ─┬─> (MMWeights ─> AMBuckets) / Sink / (TATTapPoint -> Neurons)
                                                        └─> (MMWeights ─> AMBuckets) / Sink / (TATTapPoint -> Neurons)
                                                        ⋮
                                                        └─> (MMWeights ─> AMBuckets) / Sink / (TATTapPoint -> Neurons)
        """

        if len(out_conns) == 1:
            conn = out_conns[0]
            tgt = conn.dest
            self.__connect(self._get_resource("AMBuckets", tgt)

        elif len(self.conns_out) > 1:
            #   Conn 6: Bucket ─┬─> Bucket / Output
            #                   └─> Bucket / Output
            #                   ⋮
            #                   └─> Bucket / Output

            bucket = self._get_resource("AMBuckets")
            TAT_fanout = hwr.TATFanout(self.dimensions) # create TAT fanout entries

            bucket.connect(TAT_fanout)

            # standard (str, None) key in resources for TATFanout, doesn't belong to any one connection
            self._append_resource("TATFanout", TAT_fanout) 

            for conn in out_conns:
                tgt = conn.dest
                self.__connect(self._get_resource("AMBuckets", tgt)


    # bucket can have a fanout, makes the same connections either from
    # its MMBucket, or TATFanout
    def __connect(self, src_resource, tgt):
        # Conn 4/5a: Bucket -> Bucket
        if isinstance(tgt, Bucket):
            self._connect_to_bucket(src_resource, tgt)

        # Conn 4/5b: Bucket -> TATFanout -> Output
        # could connect directly to output, but we do this to prevent clobbering
        elif isinstance(tgt, Output):

            TAT_fanout = hwr.TATFanout(self.get_num_dimensions()) # create TAT fanout
            sink = tgt._get_resource("Sink")

            # make connections
            src_resource.connect(TAT_fanout)
            TAT_fanout.connect(sink)

            # append to resources
            self._append_resource(("TATFanout", tgt), TAT_fanout)

        # Conn 4/5c: AMBuckets ─> (TATTapPoint -> Neurons)
        elif isinstance(tgt, Pool):
            src_resource.connect(tgt._get_resource("TATTapPoint"))

        else:
            raise ConnectionTypeError(src_obj, tgt)


