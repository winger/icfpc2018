
import os
import math


LOGS_DIR = 'logs/'
METADATA_DIR = "metadata/"

class Metadata:
    def __init__(self):
        self.R = 0
        self.default_energy = 0
        self.max_score = 0

class ProblemStats:
    def __init__(self):
        # map from bot tag
        self.energy = {}
        self.score = {}

        self.metadata = None

# returns Metadata
def read_metadata_file(file_name):
    f = open(file_name, 'r')
    m = Metadata()
    for s in f.readlines():
        s = s.strip('\n')
        if "=" not in s:
            continue
        # print s
        key, value = s.split('=')
        if key == "dflt_energy":
            m.default_energy = int(value)
        if key == "R":
            m.R = int(value)
        if key == "max_score":
            m.max_score = int(value)

    return m


# returns ProblemStats
def read_log_file(file_name, m):
    f = open(file_name, 'r')
    stats = ProblemStats()
    stats.m = m
    for s in f.readlines():
        s = s.strip('\n')
        if "trace=" not in s:
            continue

        parts = s.split(' ')
        # print s
        info = {}
        for part in parts:
            if "=" in part:
                # print part
                key, value = part.split('=')
                info[key] = value
        assert info["correct"] == "1"
        bot = info["trace"]
        current_energy = int(info["energy"])
        stats.energy[bot] = current_energy

        perf = 1.0 - float(current_energy) / float(m.default_energy)
        stats.score[bot] = int(1000. * perf * int(math.log(m.R) / math.log(2)))
    return stats


def find_best_bot(stats):
    best_energy = None
    best_bot = None

    for bot, energy in stats.energy.items():
        if "/proxyTracesF/" in bot:
            continue
        if best_energy is None or energy < best_energy:
            best_energy = energy
            best_bot = bot
    return best_bot, best_energy

def main():
    log_files = os.listdir(LOGS_DIR)

    stats_map = {}

    for f in sorted(log_files):
        m = read_metadata_file(METADATA_DIR + "/" + f)
        stats = read_log_file(LOGS_DIR + "/" + f, m)

        # skip reassembly problems for now
        if f.startswith("FR"):
            continue

        assert f.endswith(".txt")
        name = f[:-4]
        stats_map[name] = stats

        best_bot, best_energy = find_best_bot(stats)
        best_score = stats.score[best_bot]
        print "{} {} score={} max_score={}".format(name, best_bot, best_score, m.max_score)

if __name__ == "__main__":
    main()
