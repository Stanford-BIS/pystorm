from abc import ABC

# GraphObject-specific exceptions
class ConnectionTypeError(Exception):
    def __init__(input_obj, output_obj):
        self.input_obj = input_obj
        self.output_obj = output_obj
        self.message(str(type(self.input_obj)) + " can't be connected to " + str(type(self.output_obj)) + ".\n" +
                "tried to connect" + self.input_obj.get_label() + " to " + self.output_obj.get_label())

class FanoutError(Exception):
    def __init__(obj, fanout, max_fanout=1):
        self.obj = obj
        self.fanout = len(obj.out_conns)
        self.message(self.obj.get_label() + " of type " + type(self.obj) +
                " can only have fanout of " + str(max_fanout) + ". Tried making fanout of " + str(self.fanout))


class GraphObject(ABC):
    def __init__(self, label):
        self.label = label
        self.resources = {}
        self.out_conns = []

    @abstractmethod
    def __repr__(self):
        pass

    def __gt__(self, other_obj):
        return self.label > other_obj.label

    def get_label(self):
        return self.label

    def reinit_resources(self):
        self.resources = []

    # we want to ensure we don't clobber any resources
    def _append_resource(self, key, resource):
        if isinstance(key, str):
            key = (key, None)
        elif isinstance(key, tuple) and len(key) == 2 and 
             isinstance(key[0], str) and 
             isinstance(key[1], GraphObject))
             pass
        else
            raise TypeError("key for _append_resource must be str or (str, GraphObject)")
        
        if key in self.resources:
            print("tried to add the same resource key twice")
            print("  to ", self.label, self)
            assert(False)

        self.resources[key] = resource

    def _get_resource(self, key):
        if isinstance(key, str):
            key = (key, None)
        elif isinstance(key, tuple) and len(key) == 2 and 
             isinstance(key[0], str) and 
             isinstance(key[1], GraphObject))
             pass
        else
            raise TypeError("key for _get_resource must be str or (str, GraphObject)")
        return self.resources[key]

    @abstractmethod
    def create_intrinsic_resources(self):
        pass

    @abstractmethod
    def create_connection_resources(self):
        pass

    # helpers for create_connection_resources
    def _connect_to_bucket(self, src_resource, tgt):
        src_resource = self.resources[resource_name]

        TAT_acc = hwr.TATAccumulator(tgt.get_num_dimensions()) # create TAT acc entries
        weights = hwr.MMWeights(conn.weights) # create weights
        bucket = tgt._get_resource("AMBuckets")

        # make connections
        source_resource.connect(TAT_acc)
        TAT_acc.connect(weights)
        weights.connect(bucket)

        # append to resources
        self._append_resource(("MMWeights", tgt), weights)
        self._append_resource(("TATAccumulator", tgt), TAT_acc)

    def get_single_conn_out(self):
        if len(self.out_conns) > 1:
            raise FanoutError(self)

        conn = self.out_conns[0]
        tgt = conn.dest
        return conn, tgt

