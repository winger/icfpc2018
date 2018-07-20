#include "matrix.h"
#include "trace.h"

void TraceDecodeEncodeTest();

int main()
{
    cout << "Hello!" << endl;
    TraceDecodeEncodeTest();
    Matrix m;
    m.ReadFromFile("LA001_tgt");
    m.Print();
    Trace t;
    t.ReadFromFile("dfltTracesL/LA001.nbt");
    return 0;
}
