#include "trace.h"

void TestTraceDecodeEncode()
{
    Trace t;
    for (unsigned i = 1; i <= 186; ++i)
    {
        string si = to_string(1000 + i).substr(1);
        t.ReadFromFile("dfltTracesL/LA" + si + ".nbt");
        t.WriteToFile("testTracesL/LA" + si + ".nbt");
    }
}
