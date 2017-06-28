def Pack(parts, widths):
    assert len(parts) == len(widths)
    assert(sum(widths) < 64) # this will cause arithmetic problems
    x = long(0)
    pos = 0
    for p, w in zip(parts, widths):
        x += 2**pos * p
        pos += w
    return x

import numpy as np

def Unpack(x, fwidths):
    widths = [int(w) for w in fwidths]
    # get one more part than widths
    parts = []
    if isinstance(x, float):
        xcurr = int(x)
    if isinstance(x, np.ndarray):
        xcurr = x.astype(int)
    else:
        xcurr = x
    for idx, w in enumerate(widths):
        assert w > 0
        xlo = xcurr % 2**w
        xcurr = xcurr / 2**w
        parts += [xlo]
    parts += [xcurr]
    return parts

def get_bit(x, i):
  xs = x / 2**i
  return xs % 2

def set_bit(x, i):
  return np.bitwise_or(x, ~2**i)

def clear_bit(x, i):
  return np.bitwise_and(x, ~2**i)

def invert_bits(x, N):
  ones = np.ones_like(x).astype(int) * 2**N-1
  return np.bitwise_xor(ones, x)

def twosc2dec(x, M):
    assert x < 2**M
    if x >= 2**(M-1): # negative
        return x - 2**M
    else: # positive
        return x

def dec2onesc(x, M):
    assert np.sum(x >= 2**(M-1)) == 0
    assert np.sum(x <= -2**(M-1)) == 0
    x = np.array(x)
    xonesc = x.copy().astype(int)
    neg = x < 0
    xonesc[neg] = invert_bits(-xonesc[neg], M)
    return xonesc

def BinString2Int(xstr):
  assert(len(xstr) < 64)
  x = 0
  power = 1
  for el in xstr:
    x += power * int(el)
    power *= 2
  return x

def Int2BinString(x, N):
  xmod = x
  xstr = []
  for i in xrange(N):
    xstr += str(xmod % 2)
    xmod = xmod >> 1
  return xstr



