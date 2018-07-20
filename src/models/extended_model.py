from bitstring import ConstBitStream

from model import Model

class ExtendedModel(Model):
    """Extended model file

    https://icfpcontest2018.github.io/lgtn/task-description.html#extended-model-files"""
    def __init__(self, stream):
        super().__init__(stream)
        stream.bytealign()
        self.bots = []
        while stream.pos < stream.length:
            print("{} of {}".format(stream.pos, stream.length))
            bot, x, y, z = stream.readlist("uint:8, uint:8, uint:8, uint:8")
            self.bots += (bot, x, y, z)

if __name__ == "__main__":
    DEFAULT_MODEL = ExtendedModel(ConstBitStream(filename="problemsL/LA001_tgt.mdl"))
    print(DEFAULT_MODEL)
