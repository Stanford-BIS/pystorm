from abc import ABC, abstractmethod

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

        # Dictionary keys are of the form (resource_name, target_graph_object).
        # Resource_name is a string, and typically refers to the string __name__ of the stored Resource.
        # target_graph_object is a GraphObject or None. When the stored resource is intrinsic, or isn't
        # associated with any particular connection (e.g. TATFanout for a fanout to many outputs),
        # target_graph_object is None, otherwise, it's the GraphObject of the target connected to by the Resource
        self.resources = {}

        # outgoing connections made from this GraphObject
        self.out_conns = []

    @abstractmethod
    def __repr__(self):
        pass

    def __gt__(self, other_obj):
        return self.label > other_obj.label

    def get_label(self):
        return self.label

    def reinit_resources(self):
        self.resources = {}

    def __check_key(self, key):
        """Puts key to self.resources into proper form

        parameters:
        key: string or (string, GraphObject)

        string is converted to (string, None) (for intrinsic resources)

        returns:
        (string, GraphObject)
        """

        if isinstance(key, str):
            key = (key, None)
        elif isinstance(key, tuple) and len(key) == 2 and \
             isinstance(key[0], str) and \
             isinstance(key[1], GraphObject):
            pass
        else:
            raise TypeError("key for _append_resource must be str or (str, GraphObject)")
        return key

    # we want to ensure we don't clobber any resources
    def _append_resource(self, key, resource):
        key = self.__check_key(key)
        
        if key in self.resources:
            print("tried to add the same resource key twice")
            print("  to ", self.label, self)
            assert(False)

        self.resources[key] = resource

    def _get_resource(self, key):
        key = self.__check_key(key)

        return self.resources[key]

    @abstractmethod
    def create_intrinsic_resources(self):
        pass

    @abstractmethod
    def _connect_from(self, src, src_resource_key, conn):
        pass

    @abstractmethod
    def create_connection_resources(self):
        pass

    def _get_single_conn_out(self):
        if len(self.out_conns) > 1:
            raise FanoutError(self)

        conn = self.out_conns[0]
        tgt = conn.dest
        return conn, tgt

    def _check_conn_from_type(self, src, allowed_types):
        if type(src).__name__ not in allowed_types:
            raise ConnectionTypeError(src, self)



