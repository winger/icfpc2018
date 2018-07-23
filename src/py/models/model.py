import itertools
import sys

from bitarray import bitarray
from bithack import asint
import numpy

class Model:
    """Model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#model-files"""
    def __init__(self, data):
        self.size = asint(bitarray(data[:8], endian="big"))
        cell_count = self.size ** 3
        cells = numpy.frombuffer(data[8:8 + cell_count].unpack(), dtype=bool).astype(int)
        cells.resize(cell_count)
        self.data = cells.reshape((self.size, self.size, self.size))

    def box(self):
        x, y, z = numpy.nonzero(self.data)
        return ((x.min(), y.min(), z.min()), (x.max(), y.max(), z.max()))

    def box_size(self):
        filled = numpy.nonzero(self.data)
        return tuple(f.max() - f.min() + 1 for f in filled)

    def is_well_formed(self):
        """Validates model according to spec"""
        return True # TODO implement

    def __str__(self):
        return "Model size {}\nData {}".format(self.size, self.data)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("No file to parse")
    with open(sys.argv[1], "rb") as file:
        data = bitarray(endian="little")
        data.fromfile(file)
        DEFAULT_MODEL = Model(data)
        print(DEFAULT_MODEL)
