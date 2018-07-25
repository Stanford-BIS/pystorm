from abc import ABC, abstractmethod

# GraphObject-specific exceptions
class GraphObjectError(Exception):
    """Base class for GraphObject Exceptions"""
    name = "graph_object.py"

class ConnectionTypeError(GraphObjectError):
    """Exception to raise when connected GraphObject types are incompatible"""
    def __init__(self, input_obj, output_obj):
        self.input_obj = input_obj
        self.output_obj = output_obj
        self.message = (
            str(type(self.input_obj)) + " can't be connected to " +
            str(type(self.output_obj)) + ".\n" + "tried to connect" +
            self.input_obj.get_label() + " to " + self.output_obj.get_label())
    def __str__(self):
        return self.message

class FanoutError(GraphObjectError):
    """Exception to raise when a GraphObject's fanout is too large"""
    def __init__(self, obj, max_fanout=1):
        self.obj = obj
        self.fanout = len(obj.out_conns)
        self.max_fanout = max_fanout
        self.message = (
            self.obj.get_label() + " of type " + str(type(self.obj)) +
            " can only have max fanout of " + str(self.max_fanout) +
            " but tried making fanout of " + str(self.fanout))
    def __str__(self):
        return self.message

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
            logger.critical("tried to add the same resource key twice")
            logger.critical("  to {} {}".format(self.label, self))
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
