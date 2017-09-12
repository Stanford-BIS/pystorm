import ok
import sys
import string

bitfile = "quartus/output_files/OKCoreTestHarness.rbf"

def InitOK(bitfile):
  # Open the first device we find.
  zem = ok.okCFrontPanel()
  if (zem.NoError != zem.OpenBySerial("")):
    print ("A device could not be opened.  Is one connected?")
    return(False)

  # Get some general information about the device.
  devInfo = ok.okTDeviceInfo()
  if (zem.NoError != zem.GetDeviceInfo(devInfo)):
    print ("Unable to retrieve device information.")
    return(False)
  print("         Product: " + devInfo.productName)
  print("Firmware version: %d.%d" % (devInfo.deviceMajorVersion, devInfo.deviceMinorVersion))
  print("   Serial Number: %s" % devInfo.serialNumber)
  print("       Device ID: %s" % devInfo.deviceID)

  zem.LoadDefaultPLLConfiguration()

  # Download the configuration file.
  if (zem.NoError != zem.ConfigureFPGA(bitfile)):
    print ("FPGA configuration failed.")
    return(False)

  # Check for FrontPanel support in the FPGA configuration.
  if (False == zem.IsFrontPanelEnabled()):
    print ("FrontPanel support is not available.")
    return(False)

  print ("FrontPanel support is available.")
  return(zem)

zem = InitOK(bitfile)

def ToBytes(s):
  v = int(s, 2)
  b = bytearray()
  while v:
    b.append(v & 0xff)
    v >>= 8
  return b[::-1]

def PrintBytearrayAs32b(buf_out, ignore_nop=True):
  nop_count = 0
  for idx in xrange(len(buf_out) // 4):
    this_word_flipped = buf_out[4*idx:4*(idx+1)]
    this_word = this_word_flipped[::-1]
    if (nop_up != this_word or not ignore_nop):
      for j in xrange(4):
        to_print = ""
        elcopy = this_word[j]
        for i in xrange(8):
          to_print += str(elcopy % 2)
          elcopy = elcopy >> 1
        to_print += " "
        sys.stdout.write(str(to_print[::-1]))
      sys.stdout.write('\n')
    else:
      nop_count += 1
  print("plus " + str(nop_count) + " NOPs")


ep_in = 0x80 # BTPipeIn ep num
ep_out = 0xa0 # PipeOut ep num

nop_down = ToBytes("10" + "111111" + 23*"0" + "1") # you can pack downstream frames with this

nop_up = ToBytes("01000000" + 24*"0") # this is what an upstream nop looks like. The FPGA sends these if the host requests data and the FPGA has none

buf_out = bytearray(128)
zem.ReadFromPipeOut(ep_out, buf_out)

PrintBytearrayAs32b(buf_out)

