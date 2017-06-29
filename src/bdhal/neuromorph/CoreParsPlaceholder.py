def CoreParsPlaceholder():

    # XXX these are placeholder values (for a very small Core): should get these values from BDPars.h
    pars = {}
    pars['MM_height'] = 8
    pars['MM_width'] = 8
    pars['AM_size'] = 16
    pars['TAT_size'] = 32
    pars['NeuronArray_height'] = 8
    pars['NeuronArray_width'] = 8
    pars['NeuronArray_pool_size'] = 4
    pars['num_threshold_levels'] = 8
    pars['min_threshold_value'] = 64
    pars['max_weight_value'] = 127
    pars['NeuronArray_neurons_per_tap'] = 4

    return pars
