import sys
from binascii import b2a_hex


def PrintBytearrayAs32b(buf_out):
    nop_count = 0
    for idx in range(len(buf_out) // 4):
        this_word_flipped = buf_out[4 * idx:4 * (idx + 1)]
        this_word = this_word_flipped[::-1]
        if (this_word[0] == 64 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0):
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


def PrettyPrintBytearray(buf_in, grouping=4):
    num_words = int(len(buf_in) // 4)
    op_stream = []
    frmt = ("{0:0%db}" % (grouping * 8))
    for idx in range(num_words):
        _bytes = buf_in[idx * grouping : (idx + 1) * grouping][::-1]
        _bytes = frmt.format(int(b2a_hex(_bytes), 16))
        print(_bytes)
