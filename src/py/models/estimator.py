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
    names = sorted([name[-11:-8] for name in glob(join(path, "FR*_src.mdl"))])
    
    for name in names:
        source = join(path, "FR{}_src.mdl".format(name))
        target = join(path, "FR{}_tgt.mdl".format(name))

        with open(source, "rb") as s, open(target, "rb") as t:
            sdata = bitarray(endian="little")
            sdata.fromfile(s)
            src = Model(sdata)
            tdata = bitarray(endian="little")
            tdata.fromfile(t)
            tgt = Model(tdata)
            diff = np.absolute(tgt.data - src.data)
            changed = np.sum(diff)
            perc = changed / (tgt.size ** 3)
            print("Task {} {:.2%} different, {} of {} changed".format(name, perc, changed, tgt.size ** 3))