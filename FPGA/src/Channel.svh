`ifndef CHANNEL_SVH
`define CHANNEL_SVH

// valid/data-acknowledge channel
//
// If valid (.v) goes high sometime in clock cycle i,
// that means that the data has a new, valid value on the next clock edge
// (the beginning of cycle i+1). 
// For full throughput, the receiver will assert acknowledge (.a) sometime
// after .v, still in clock cycle i. The receiver samples the
// data on the next clock edge (the beginning of cycle i+1).
// This scheme allows for full speed (one data transfer per cycle).
//
// Practically speaking, there's nothing preventing .v/.d from being
// register outputs, but they're often combinational functions of FSM states, 
// so I've marked them as transitioning a little after the clock edge in the
// following diagrams. There's nothing that says that .a can't be a register
// output, but for full throughput, it needs to be a purely combinational
// function of .v
//
// Timing example:
//
// .v |  ______|________|________|________|_       |
//    |_|      |        |        |        | |______|
//    |        |        |        |        |        |
// .d |_  _____|  ______|________|  ______|  ______|
//    |X\/  1  |\/   2  |   2    |\/   3  |\/   X  |
//    |_/\_____|/\______|________|/\______|/\______|
//    |        |        |        |        |        |
// .a |   _____|__      |   _____|________|__      |
//    |__|     |  |_____|__|     |        |  |_____|
//    |        |        |        |        |        |
//
//  1.receiver         3.then reads twice
//    reads once         (full throughput)
//              
//            2.then can't                 4.sender has
//              read for some                no more data
//              reason, doesn't ack
//
// Notes:
//
// .v |_   ____|________|
//    | |_|    |        |
//    |        |        |
// .d |________|  ______|
//    |   1    |\/   2  |
//    |________|/\______|
//    |        |        |
// .a |   _   _|________|
//    |__| |_| |        |
//    |        |        |
//
//    Technically the above is legal. We're not doing async handshakes with phases.
//    .a and .d just have to be high by the next clock edge
//    It's up to the synthesis tool to make sure everything works out.
//
// .v |        |  ______|
//    |________|_|      |
//    |        |        |
// .d |________|  ______|
//    |   X    |\/   1  |
//    |________|/\______|
//    |        |        |
// .a |   _____|__      |
//    |__|     |  |_____|
//    |        |        |
//
//    This, however, breaks the protocol.
//    .a should never be high on a clock edge that .v isn't high on
//
// .v |  ______|________|_       |
//    |_|      |        | |______|
//    |        |        |        |
// .d |_  _____|  ______|  ______|
//    |X\/  1  |\/   2  |\/   X  |
//    |_/\_____|/\______|/\______|
//    |        |        |        |
// .a |   _____|__      |        |
//    |__|     |  |_____|________|
//    |        |        |        |
//
//    It's legal for .v to go away without being acknowledged
//    (But I don't think I ever do this).
//    This only works if .a if being generated combinationally.
//
// .v |  ______|________|________|________|
//    |_|      |        |        |        |
//    |        |        |        |        |
// .d |_  _____|________|  ______|________|
//    |X\/  1  |    1   |\/   2  |    2   |
//    |_/\_____|________|/\______|________|
//    |        |        |        |        |
// .a |        |________|        |________|
//    |________|        |________|        |
//    |        |        |        |        |
//
//    If you know that .v isn't going to drop without being acked
//    (like it does in the previous example), there's nothing keeping
//    you from generating .a as a register output.
//    You're just never going to get more than half throughput.
//
interface Channel #(parameter N = -1);
  logic [N-1:0] d;
  logic v;
  logic a;
endinterface

// bundles several channels. There are some syntactic issues with
// arrays of Channels in modules ports.
interface ChannelArray #(parameter N = -1, parameter M = -1);
  logic [M-1:0][N-1:0] d;
  logic [M-1:0] v;
  logic [M-1:0] a;
endinterface

// valid/data-acknowledge channel, but no data
// used for a synchronization handshake
interface DatalessChannel;
  logic v;
  logic a;
endinterface

`endif
