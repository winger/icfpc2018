#include "matrix.h"
#include "trace.h"

int main()
{
    cout << "Hello!" << endl;
    Matrix m;
    m.ReadFromFile("LA001_tgt");
    m.Print();
    Trace t;
    t.ReadFromFile("dfltTracesL/LA001.nbt");
    return 0;
}
