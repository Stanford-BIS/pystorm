import argparse
import numpy as np
import sys

from pystorm.hal import HAL
HAL = HAL()

parser = argparse.ArgumentParser(description='register a chip in the calibration database')
parser.add_argument('chip_name', type=str, help='the name you want to assign to the attached chip')
args = parser.parse_args()

chip_name = args.chip_name

act = HAL.get_unique_chip_activation()

if np.sum(act > 0) < 10:
    print("got no (or very little) activity. Chip crashed? Try again.")
    sys.exit(0)

curr_chip_name, sims = HAL.cdb.find_chip(act)
print("inner products to chips currently in DB were:"
print(np.sort(sims)[::-1])

if curr_chip_name is None:
    HAL.cdb.add_new_chip(act, chip_name, commit_now=True)
    print("chip", chip_name, "registered.", np.sum(act > 0), "neurons fired")
else:
    if curr_chip_name != chip_name:
        print("chip activation already in DB with DIFFERENT name:", curr_chip_name,
                ". Inner product was", np.max(sims), "Nothing done.")
    else:
        print("chip activation already in DB. Nothing done.")

