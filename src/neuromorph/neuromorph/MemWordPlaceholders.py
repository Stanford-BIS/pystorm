from . MemWordEnums import *

# XXX this isn't permanent implementation
# just meant to behave like the actual boost object would
# the boost objects for the C++ BDWord should use the same syntax as this

class BDWord(object):
    # XXX for the boost object can determine which <template> to call based on
    # type of field_vals keys
    def __init__(self, field_vals):
        self.field_vals = field_vals

        # Core needs these to infer entry type, set them automatically like C++ object would
        if len(self.field_vals) > 0:
            if isinstance(list(self.field_vals.keys())[0], TATAccField):
                self.field_vals[TATAccField.FIXED_0] = 0
            if isinstance(list(self.field_vals.keys())[0], TATSpikeField):
                self.field_vals[TATSpikeField.FIXED_1] = 1
            if isinstance(list(self.field_vals.keys())[0], TATTagField):
                self.field_vals[TATTagField.FIXED_2] = 2

    # XXX for the boost object can determine which <template> to call based on
    # type of field_vals keys
    def At(self, field):
        if field in self.field_vals:
            return self.field_vals[field]
        else: # field not set yet, would return 0
            return 0

    # XXX unimplemented calls, shouldn't be needed:
    # init(int)
    # AsUint()


