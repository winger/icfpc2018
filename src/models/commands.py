from bitarray import bitarray
from bithack import asint

import numpy as np

class Command:
    def __init__(self, array, pos):
        pass

    def __str__(self):
        return self.__class__.__name__

    def __len__(self):
        return 8

    def cost(self):
        return 0

    @staticmethod
    def parse_command(array, pos):
        for c in Command.__subclasses__():
            if c.is_command(array, pos):
                return c(array, pos)
        return None

    @classmethod
    def is_command(cls, array, pos):
        return False

class Halt(Command):
    """Halt the execution"""
    @classmethod
    def is_command(cls, array, pos):
        return array[pos:pos + 8] == bitarray("11111111")

class Wait(Command):
    """Wait for next command"""
    @classmethod
    def is_command(cls, array, pos):
        return array[pos:pos + 8] == bitarray("11111110")

class Flip(Command):
    """Flip the harmony"""
    @classmethod
    def is_command(cls, array, pos):
        return array[pos:pos + 8] == bitarray("11111101")

class SMove(Command):
    """Straight Move"""
    def __init__(self, array, pos):
        super(SMove, self).__init__(array, pos)
        self.move = np.zeros(3)
        self.move[asint(array[pos + 2:pos + 4]) - 1] = asint(array[pos + 11:pos + 16]) - 15

    def __str__(self):
        return "{} {}".format(self.__class__.__name__, self.move)

    def __len__(self):
        return 16

    def cost(self):
        return 2 * np.absolute(self.move).sum()

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 4:pos + 8] == bitarray("0100")

class LMove(Command):
    """L Move"""
    def __init__(self, array, pos):
        super(LMove, self).__init__(array, pos)
        self.move = np.zeros((2, 3))
        self.move[0, asint(array[pos + 2:pos + 4]) - 1] = asint(array[pos + 12:pos + 16]) - 5
        self.move[1, asint(array[pos:pos + 2]) - 1] = asint(array[pos + 8:pos + 12]) - 5

    def __str__(self):
        return "{} {}".format(self.__class__.__name__, self.move)

    def __len__(self):
        return 16

    def cost(self):
        return 2 *  np.absolute(self.move).sum() + 4

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 4:pos + 8] == bitarray("1100")

class FusionP(Command):
    """Fusion Primary"""
    def __init__(self, array, pos):
        super(FusionP, self).__init__(array, pos)
        packed = asint(array[pos:pos + 5])
        self.dir = np.zeros(3)
        for i in reversed(range(3)):
            self.dir[i] = packed % 3 - 1
            packed //= 3

    def __str__(self):
        return "{} {}".format(self.__class__.__name__, self.dir)

    def cost(self):
        return -24

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 5:pos + 8] == bitarray("111")

class FusionS(Command):
    """Fusion Secondary"""
    def __init__(self, array, pos):
        super(FusionS, self).__init__(array, pos)
        packed = asint(array[pos:pos + 5])
        self.dir = np.zeros(3)
        for i in reversed(range(3)):
            self.dir[i] = packed % 3 - 1
            packed //= 3

    def __str__(self):
        return "{} {}".format(self.__class__.__name__, self.dir)

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 5:pos + 8] == bitarray("110")

class Fission(Command):
    """Fission"""
    def __init__(self, array, pos):
        super(Fission, self).__init__(array, pos)
        packed = asint(array[pos:pos + 5])
        self.dir = np.zeros(3)
        for i in reversed(range(3)):
            self.dir[i] = packed % 3 - 1
            packed //= 3
        self.size = asint(array[pos + 8:pos + 16])

    def __str__(self):
        return "{} {} {}".format(self.__class__.__name__, self.dir, self.size)

    def __len__(self):
        return 16

    def cost(self):
        return 24

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 5:pos + 8] == bitarray("101")

class Fill(Command):
    """Fill"""
    def __init__(self, array, pos):
        super(Fill, self).__init__(array, pos)
        packed = asint(array[pos:pos + 5])
        self.dir = np.zeros(3)
        for i in reversed(range(3)):
            self.dir[i] = packed % 3 - 1
            packed //= 3

    def __str__(self):
        return "{} {}".format(self.__class__.__name__, self.dir)

    @classmethod
    def is_command(cls, array, pos):
        return array[pos + 5:pos + 8] == bitarray("011")

if __name__ == "__main__":
    command = Command.parse_command(bitarray("1110110001110011"), 0)
    print(command.cost())
