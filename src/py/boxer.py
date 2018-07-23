from os.path import isfile, join
from glob import glob
from bitarray import bitarray
from model import Model
import numpy as np
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit("No dir to parse")
    path = sys.argv[1]
    names = sorted([name[-11:-8] for name in glob(join(path, "FD*_src.mdl"))])

    result = {}
    
    for name in names:
        source = join(path, "FD{}_src.mdl".format(name))

        with open(source, "rb") as s:
            sdata = bitarray(endian="little")
            sdata.fromfile(s)
            src = Model(sdata)
            print("Task {}: box size {}".format(name, src.box_size()))
            result[name] = src.box_size()

    print("Less than 30 ** 3")
    for name, box in result.items():
        if all(x <= 30 for x in box):
            print("{} - {}".format(name, box))
    
    print("Less than 30 x and z:")
    for name, box in result.items():
        x, y, z = box
        if x <= 30 and y > 30 and z <= 30:
            print("{} - {}".format(name, box))