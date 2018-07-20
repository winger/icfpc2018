#include "matrix.h"
#include "trace.h"

void TestTraceDecodeEncode();

int main()
{
    cout << "Hello!" << endl;
    TestTraceDecodeEncode();
    Matrix m;
    m.ReadFromFile("LA001_tgt");
    m.Print();
    Trace t;
    t.ReadFromFile("dfltTracesL/LA001.nbt");
    return 0;
}
