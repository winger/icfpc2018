import sys

from bitarray import bitarray, bits2bytes
import numpy

from model import Model

class ExtendedModel(Model):
    """Extended model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#extended-model-files"""
    def __init__(self, data):
        super().__init__(data)
        bots_data = bitarray(data[bits2bytes(self.size ** 3 + 8) * 8:], endian="big")
        bots = numpy.frombuffer(bots_data.tobytes(), dtype=numpy.int8)
        self.bots = bots.reshape((-1, 4))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("No file to parse")
    with open(sys.argv[1], "rb") as file:
        data = bitarray(endian="little")
        data.fromfile(file)
        DEFAULT_MODEL = ExtendedModel(data)
        print(DEFAULT_MODEL.bots)
