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
      word = ""
      for j in range(4):
        to_print = ""
        elcopy = this_word[j]
        for i in range(8):
          to_print += str(elcopy % 2)
          elcopy = elcopy >> 1
        word += str(to_print[::-1])
        # sys.stdout.write(str(to_print[::-1]))
      word = word[2:]
      word = ' '.join(word[i:i+10] for i in range(0, len(word), 10))
      print(word)
      # sys.stdout.write('\n')
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


codes = ["0b00001000000010011000111000011110", "0b00010000000011110000111000110010"]


outputs = []
# for n in range(8): #range(int(block_size / len(codes))):
#   word = ""
#   word+="0b000000000000000000000000"
#   for m in range(n-1):
#     word+="0"
#   word+="1"
#   for m in range(7-n):
#     word+="0"
#   outputs.append(FormatBits(word))
#   outputs.append(FormatBits(codes[1])) #add route

for n in range(2):
  for code in codes:
    outputs.append(FormatBits(code))

while len(outputs) < block_size:
  outputs.append(FormatBits(nop_down))

buf = bytearray()
for el in outputs:
    buf.extend(el)

print("============SENDING=============")
PrintBytearrayAs32b(buf)
print("================================")

# for n in range(3000):
  # sys.stdout.write(str(n) + " ")
written = dev.WriteToBlockPipeIn(ep_dn, block_size, buf)
sys.stdout.write('\n')
# print("Written " + str(written))

time.sleep(2)
i=0
for j in range(3):
    # get the memory programming read and one output!
    out_buf = bytearray(block_size*4)
    dev.ReadFromBlockPipeOut(ep_up, block_size*4 ,out_buf)
    print("------------RECEIVED------------")
    PrintBytearrayAs32b(out_buf)
    print("--------------------------------")
    print(i)
    time.sleep(1)
    i += 1
