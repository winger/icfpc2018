#include "relayers_base.h"

ReassemblySolverLayersBase::ReassemblySolverLayersBase(const Matrix& s, const Matrix& t, bool levitation)
    : SolverBase(s, levitation), source(s), target(t) {}

bool ReassemblySolverLayersBase::NeedChange(const Coordinate& c) const {
    return state.matrix.Get(c) != target.Get(c);
}

size_t ReassemblySolverLayersBase::GreedyFill(const Coordinate& c0, bool dry, size_t& count) {
    size_t result = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                Coordinate c = {c0.x + dx, c0.y + dy, c0.z + dz};
                CoordinateDifference cd = c - c0;
                if (matrix.IsInside(c) && cd.IsNearCoordinateDifferences() && NeedChange(c)) {
                    ++result;
                    if (!dry) {
                        assert(GetBotPosition() == c0);
                        if (!state.matrix.Get(c)) {
                            Command m(Command::Fill);
                            m.cd1 = cd;
                            AddCommand(m);
                        } else {
                            Command m(Command::Void);
                            m.cd1 = cd;
                            AddCommand(m);
                        }
                        assert(!NeedChange(c));
                        assert(count != 0);
                        --count;
                    }
                }
            }
        }
    }
    return result;
}

size_t ReassemblySolverLayersBase::GreedyReassemble(size_t& count) {
    Coordinate bestCoordinate = {-1, -1, -1};
    static constexpr int INF_ESTIMATION = -1000000;
    int bestEstimation = INF_ESTIMATION;
}

void ReassemblySolverLayersBase::Solve(Trace& output) {
    SolveInit();

    output.tag = "reassembly";

    size_t count = 0;
    for (int x = 0; x < source.GetR(); ++x) {
        for (int y = 0; x < source.GetR(); ++x) {
            for (int z = 0; x < source.GetR(); ++x) {
                if (NeedChange({x, y, z})) {
                    ++count;
                }
            }
        }
    }

    while (count) {
        if (!GreedyReassemble(count)) {
            throw StopException();
        }
    }

    SolveFinalize();
    output = state.trace;
}

Evaluation::Result ReassemblySolverLayersBase::Solve(const Matrix& source, const Matrix& target, Trace& output, bool levitation) {
    ReassemblySolverLayersBase solver(source, target, levitation);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);

}
