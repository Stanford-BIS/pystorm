"""
Measure Synapse's time-constant

- Hit Synapse with multiple-spikes to saturate the synapse
    - Use very high duty-cycle ON pulse for PE
- Stop the input to synapse
- Measure the output spike rate decay
    - Soma should be high-firing rate AND sensitive

Prerequisities

- Get Soma F-F curve to linear input
    - I_syn O{ t_on / (t_on + t_off) = t_on / T_in if (t_on + t_off) < T_in and t_off << T_in
    - Sweep T_in with large EXC-DC to measure F-F
    - Use the same EXC, DC to measure Synapse's time-constant
"""


