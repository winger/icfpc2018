import itertools

from bitstring import ConstBitStream
import numpy

class Model:
    """Model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#model-files"""
    def __init__(self, stream):
        self.size = stream.read(8).uint
        self.data = numpy.zeros((self.size, self.size, self.size))
        for cell in itertools.product(range(self.size), repeat=3):
            self.data[cell] = stream.read("bool")

    def is_well_formed(self):
        """Validates model according to spec"""
        return True # TODO implement

    def __str__(self):
        return "Model size {}\nData {}".format(self.size, self.data)

if __name__ == "__main__":
    DEFAULT_MODEL = Model(ConstBitStream(filename="problemsL/LA001_tgt.mdl"))
    print(DEFAULT_MODEL)
