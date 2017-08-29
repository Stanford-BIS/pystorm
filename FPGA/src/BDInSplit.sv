`include "Channel.svh"

module BdInSplit #(parameter NBDdata_in) (
  Channel tag_out,
  Channel other_out,
  Channel BD_in);

// SF_PCPacker split, built of two 2-way splits and a merge

// look at the two MSBs
// if they are "00" or "01", it's a a tag
  
// intermediate channels
Channel #(NBDData_in) c_tat();
Channel #(NBDData_in) c_acc();
Channel #(NBDData_in) c_intermediate();

ChannelSplit #(.Mask(2**(NBDdata_in-1) + 2**(NBDdata-2)), .Code0(0))
 split0(c_tat, c_intermediate, BD_in);

ChannelSplit #(.Mask(2**(NBDdata_in-1) + 2**(NBDdata-2)), .Code0(2**(NBDdata-2))
 split0(c_acc, other_out, c_intermediate);

ChannelMerge merge(tag_out, c_tat, c_acc);

endmodule
