from graph_object import GraphObject, ConnectionTypeError, FanoutError
import pystorm.hal.neuromorph.hardware_resources as hwr

class Input(GraphObject):
    def __init__(self, label, dimensions):
        super(GraphObject, self).__init__(label)
        self.dimensions = dimensions

    def __repr__(self):
        return "Input " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("Source", hwr.Source(self.dimensions))

    def create_connection_resources(self):
        """
        Conn 1:
              neuromorph graph: Input ─> Pool
            hardware_resources: Source ─> (TATTapPoint ─> Neurons)
        Conn 2:
              neuromorph graph: Input ─> Bucket
            hardware_resources: Source ─> TATAcuumulator ─> MMWeights ─> AMBuckets
        """
        conn, tgt = self.get_single_conn_out()

        # Conn 1: Input -> Pool
        if isinstance(tgt, Pool):
            self._get_resource("Source").connect(tgt._get_resource("TATTapPoint"))

        # Conn 2: Input -> Bucket
        elif isinstance(tgt, Bucket):
            self._connect_to_bucket(self._get_resource("Source"), tgt)

        else:
            raise ConnectionTypeError(self, tgt)

    def get_generator_idxs(self):
        return self._get_resource("Source").generator_idxs

    def get_generator_out_tags(self):
        return self._get_resource("Source").out_tags


