from .graph_object import GraphObject
import pystorm.hal.neuromorph.hardware_resources as hwr

class Input(GraphObject):
    def __init__(self, label, dimensions):
        super(Input, self).__init__(label)
        self.dimensions = dimensions

    def __repr__(self):
        return "Input " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def create_intrinsic_resources(self):
        self._append_resource("Source", hwr.Source(self.dimensions))

    def create_connection_resources(self):
        if (len(self.out_conns) > 0):
            conn, tgt = self._get_single_conn_out()
            tgt._connect_from(self, "Source", conn)

    def _connect_from(self, src, src_resource_key, conn):
        self._check_conn_from_type(src, []) # can't connect to an input
        
    @property
    def generator_idxs(self):
        return self._get_resource("Source").generator_idxs
    @property
    def generator_out_tags(self):
        return self._get_resource("Source").out_tags

    @property 
    def core_id(self):
        return self._get_resource("Source").core_id

