import itertools
import sys

from bitarray import bitarray
import numpy

class Model:
    """Model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#model-files"""
    def __init__(self, data):
        size = bitarray(data[:8], endian="big")
        self.size = int(size.tobytes()[0])
        cells = numpy.frombuffer(data[8:8 + self.size ** 3].unpack(), dtype=bool).astype(int)
        self.data = cells.reshape((self.size, self.size, self.size))

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
