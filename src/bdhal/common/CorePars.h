#ifndef COREPARS_H
#define COREPARS_H

namespace pystorm {
namespace bdhal {

enum class CoreParsIndex {
    MM_height,
    MM_width,
    AM_size,
    TAT_size,
    NeuronArray_height,
    NeuronArray_width,
    NeuronArray_pool_size,
    num_threshold_levels,
    min_threshold_value,
    max_weight_value,
    NeuronArray_neurons_per_tap
};

typedef std::map<CoreParsIndex,uint32_t> CorePars;

} // namespace bdhal
} // namespace pystorm

#endif
