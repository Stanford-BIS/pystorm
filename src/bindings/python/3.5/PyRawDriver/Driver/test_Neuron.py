import sys
import time
import PyRawDriver as bd


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


driver = bd.Driver()
driver.InitBD()

driver.SetSpikeDumpState(1)
driver.SetSpikeDumpState(1)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 512)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 512)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
time.sleep(1)

#for _idx in range(4096):
#    driver.EnableSoma(_idx)
#    driver.SetSomaGain(_idx, bd.SomaGainId.ONE)
#    driver.SetSomaOffsetSign(_idx, bd.SomaOffsetSignId.POSITIVE)
#    driver.SetSomaOffsetMultiplier(_idx, bd.SomaOffsetMultiplierId.ZERO)


while(1):
    print(PrintBytearrayAs32b(driver.ReceiveWords()))
    time.sleep(1)