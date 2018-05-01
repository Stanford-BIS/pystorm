BrainDrizzle is a system that allows up to 31 BrainDrops to be networked together, and host networks with over 100,000 neurons.

#Hardware

##Boards

The BrainDrizzle board is a PCB that hosts an Altera MAX10 FPGA, a BrainDrop, and the requisite power hardware for the two chips. The board supports JTAG chaining, so all of the BrainDrizzles in the stack can be programmed at once. There is a special top board, which has a power connector and additonal hardware for JTAG, as well as all of the components of a regular BrainDrizzle board. The board communicate using a mezzanine connector. The pin-diagram will hopefully be written here at some point


Currently, the BrainDrizzle boards sit atop an Opal Kelly host board.

Board layouts can be found in the lab drobbox somewhere.

##FPGA HDL

Each BrainDrizzle FPGA contains two major logical blocks: the Core and the Router. The Core documentation exists on the wiki, so I'm not going to go into detail on it here.

###Core

The major changes to the core are the new handling of global routes and timing.

####Global Routes

Tags with global routes are packaged as input tags for other Cores, and those global tags are converted to routes. Global tags programmed to cores, like global routes, are relative.

####Timing

Heartbeats are no longer handled in the Core module. Now, the heartbeat generation and timestamps exist in OKIFC. Each board has an internal timing module for its Spike Generators and Spike Filters, but these internal timing modules do not produce heartbeats.

###Router
The Router consists of a series of handshakers, FIFOs, and serializers to control input to and output from the Core. BZSerializer and BZDeserializer convert the 27-bit Core output formats to 3 10 bit router packets, as well as prepending a packet containing the route. Routes are relative -- they represent "number of hops to reach destination".

Routers coming out of the Core are signed. Positive routes mean up (away from the host core), and negative routes mean down (towards the host core). These routes are converted into unsigned numbers in the router. In the router, where packets can only be travelling in one direction (up or down), all routes are treated as unsigned.

The "go home" route is currently negative 32. This causes the packets to come out of the bottom of the host board's router, which is where the OKIFC lives, to convert the packets to a PC-readible format.

A block diagram may go here eventually, but I'm also very lazy and that sounds hard.

#Software

Both the low level driver stack and the HAL had to change to support multi-core processing.

##Driver

The encoder, decoder, and main driver had to change slightly to allow for multiple cores.

###Encoder

The encoder is now able to generate routes from core_id's using a toRoute function. These routes are then packed into FPGA words and sent to the Comm module.

###Decoder

The decoder now has an 2-dimenstional map of output buffers, indexed by endpoint code, then by core.

###Driver

The Driver now takes in a core_id as an argument for all necessary funtions. BDPars now contains a NUM_CORES parameter, as well as extra timing parameters per core.

##HAL

The HAL had minor changes in hal.py itself, and major changes in network.py.

###HAL Changes

HAL functions were updated to take actions across multiple cores. The CORE_ID parameter was replaced with a NUM_CORES parameter.

###Network Changes

The network now uses a python wrapper for METIS to partition multi-core networks across cores. It does this in a series of steps. First, it divides the graph of hardware resources based on what physcially must be on the same core. Then, it assigns weights to those nodes and the connections between them based on the number of neurons and accumulator memory entries in each node. This new graph is run through METIS to partition it across the requested number of cores. TATFanouts are added where necessary for off-core connections. The new array of cores can then be mapped onto BrainDrizzle.