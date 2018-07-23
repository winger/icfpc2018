#include "evaluation.h"

#include "state.h"

Evaluation::Result Evaluation::Evaluate(const Matrix& source, const Matrix& target, const Trace& t)
{
    State s;
    s.Init(source, t);
    s.Run();
    // cout << s.matrix << endl;

    bool correct = (s.IsCorrectFinal() && (s.matrix == target));
    for (int x = 0; x < s.matrix.GetR(); ++x) {
        for (int y = 0; y < s.matrix.GetR(); ++y) {
            for (int z = 0; z < s.matrix.GetR(); ++z) {
                if (s.matrix.Get(x, y, z)) {
                    cout << x << " " << y << " " << z << "\n";
                }
            }
        }
    }
    return Result(correct, s.energy, source.GetR());
}
