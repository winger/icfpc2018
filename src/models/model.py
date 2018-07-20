import itertools

from bitstring import BitStream
import numpy

class Model:
    """Model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#model-files"""
    def __init__(self, stream):
        self.size = stream.read("uint:8")
        self.data = numpy.zeros((self.size, self.size, self.size))
        for i in range(8, 8 + self.size ** 3, 8):
            stream.reverse(i, i + 8)
        for cell in itertools.product(range(self.size), repeat=3):
            self.data[cell] = stream.read("bool")

    def is_well_formed(self):
        """Validates model according to spec"""
        return True # TODO implement

    def __str__(self):
        return "Model size {}\nData {}".format(self.size, self.data)

if __name__ == "__main__":
    DEFAULT_MODEL = Model(BitStream(filename="problemsL/LA001_tgt.mdl"))
    print(DEFAULT_MODEL)
