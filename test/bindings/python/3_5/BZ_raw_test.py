import PyOK as ok

import time
import sys


def FormatBits(bitstr):
    return int(bitstr, 0).to_bytes(4, byteorder='little')

# pretty print of a bytearray, ignores upstream nops
def PrintBytearrayAs32b(buf_out):
  nop_count = 0
  for idx in range(len(buf_out) // 4):
    this_word_flipped = buf_out[4*idx:4*(idx+1)]
    this_word = this_word_flipped[::-1]
    if (this_word[0] == 64 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0):
      nop_count += 1
    elif (this_word[0] == 128 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0):
      nop_count += 1
    else:
      for j in range(4):
        to_print = ""
        elcopy = this_word[j]
        for i in range(8):
          to_print += str(elcopy % 2)
          elcopy = elcopy >> 1
        to_print += " "
        sys.stdout.write(str(to_print[::-1]))
      sys.stdout.write('\n')
  print("plus " + str(nop_count) + " NOPs")

bitfile = "/home/zach/pystorm/FPGA/quartus/output_files/BZ_host_core.rbf"
block_size = 16
# for padding downstream block transmissions
nop_down = "0x80000000" # that's 1 followed by a 31 0s

dev = ok.InitOK(bitfile)
print("============STARTING=============")
time.sleep(2)

ep_dn = 0x80 # BTPipeIn ep num
ep_up = 0xa0 # PipeOut ep num


codes = ["0b01001100011100001111000001111100", "0b00000000000000000000000000000001"]


outputs = []
for n in range(2): #range(int(block_size / len(codes))):
	for code in codes:
		outputs.append(FormatBits(code))

#for code in codes:
#	outputs.append(FormatBits(code))

while len(outputs) < block_size:
	outputs.append(FormatBits(nop_down))

buf = bytearray()
for el in outputs:
    buf.extend(el)

print("============SENDING=============")
PrintBytearrayAs32b(buf)
print("================================")

written = dev.WriteToBlockPipeIn(ep_dn, block_size, buf)
# print("Written " + str(written))

time.sleep(2)
i=0
for j in range(3):
    # get the memory programming read and one output!
    out_buf = bytearray(block_size*4)
    dev.ReadFromPipeOut(ep_up, out_buf)
    print("------------RECEIVED------------")
    PrintBytearrayAs32b(out_buf)
    print("--------------------------------")
    print(i)
    time.sleep(1)
    i += 1
