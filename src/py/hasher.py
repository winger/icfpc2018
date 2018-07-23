from os.path import isfile, join
from glob import glob
from collections import defaultdict
import sys
import hashlib

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit("No dir to parse")
    path = sys.argv[1]
    BUF_SIZE = 65536

    assembly = defaultdict(lambda: "UNKNOWN")
    disassembly = defaultdict(lambda: "UNKNOWN")
    
    print("Assembly tasks:")
    for file in sorted(glob(join(path, "FA*_tgt.mdl"))):
        name = file[-11:-8]
        with open(file, "rb") as s:
            md5 = hashlib.md5()
            while True:
                data = s.read(BUF_SIZE)
                if not data:
                    break
                md5.update(data)
            
            print("Task {}: hash {}".format(name, md5.hexdigest()))
            assembly[md5.hexdigest()] = name

    print("Disassembly tasks:")
    for file in sorted(glob(join(path, "FD*_src.mdl"))):
        name = file[-11:-8]
        with open(file, "rb") as t:
            md5 = hashlib.md5()
            while True:
                data = t.read(BUF_SIZE)
                if not data:
                    break
                md5.update(data)
            
            print("Task {}: hash {}".format(name, md5.hexdigest()))
            disassembly[md5.hexdigest()] = name

    print("Resassembly tasks:")
    reuse = 0
    total = 0
    assembly_reuse = 0
    disassembly_reuse = 0
    for file in sorted(glob(join(path, "FR*_src.mdl"))):
        name = file[-11:-8]
        with open(file, "rb") as s, open(join(path, "FR{}_tgt.mdl".format(name)), "rb") as t:
            md5 = hashlib.md5()
            while True:
                data = s.read(BUF_SIZE)
                if not data:
                    break
                md5.update(data)
            src = md5.hexdigest()
            
            md5 = hashlib.md5()
            while True:
                data = t.read(BUF_SIZE)
                if not data:
                    break
                md5.update(data)
            tgt = md5.hexdigest()

            print("Task {}: from D{} to A{}".format(name, disassembly[src], assembly[tgt]))
            if disassembly[src] != "UNKNOWN" and assembly[tgt] != "UNKNOWN":
                reuse += 1
            if disassembly[src] != "UNKNOWN":
                disassembly_reuse += 1
            if assembly[tgt] != "UNKNOWN":
                assembly_reuse += 1
            total += 1

    print("{} of {} totally reusable, reusability {:.2%}".format(reuse, total, reuse / total))
    print("{} of {} disassembly reusable, reusability {:.2%}".format(disassembly_reuse, total, disassembly_reuse / total))
    print("{} of {} assembly reusable, reusability {:.2%}".format(assembly_reuse, total, assembly_reuse / total))
