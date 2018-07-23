#include "relayers_base.h"
#include "../distance_calculator.h"

ReassemblySolverLayersBase::ReassemblySolverLayersBase(const Matrix& s, const Matrix& t, bool levitation)
    : SolverBase(s, levitation), source(s), target(t) {
    state.matrix = s;
}

bool ReassemblySolverLayersBase::NeedChange(const Coordinate& c) const {
    return state.matrix.Get(c) != target.Get(c);
}

size_t ReassemblySolverLayersBase::GreedyFill(const Coordinate& c0, bool dry, size_t& count) {
    size_t result = 0;
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dz = -2; dz <= 2; ++dz) {
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

void ReassemblySolverLayersBase::MoveToCoordinateBFS(const Coordinate& c, bool finalize) {
    auto path = state.matrix.BFS(GetBotPosition(), c);
    for (const auto& p: path) {
        Command c(Command::SMove);
        c.cd1 = p;
        assert(state.matrix.IsInside(GetBotPosition() + p));
        assert(!state.matrix.Get(GetBotPosition() + p));
        AddCommand(c);
    }
    assert(GetBotPosition() == c);
}

size_t ReassemblySolverLayersBase::GreedyReassemble(size_t& count) {
    Coordinate bestCoordinate = {-1, -1, -1};
    static constexpr int INF_ESTIMATION = -1000000;
    int bestEstimation = INF_ESTIMATION;

    CoordinateSet candidates;
    state.matrix.DFS(GetBotPosition(), candidates);

    for (const auto& c : candidates) {
        size_t dummy = 0;
        int estimation = 3 * GreedyFill(c, true, dummy);
        if (estimation) {
            estimation -= MoveEnergy(GetBotPosition(), c);
            if (estimation > bestEstimation) {
                bestEstimation = estimation;
                bestCoordinate = c;
            }
        }
    }

    if (bestEstimation == INF_ESTIMATION) {
        return 0;
    }

    MoveToCoordinateBFS(bestCoordinate);
    return GreedyFill(GetBotPosition(), false, count);
}

void ReassemblySolverLayersBase::SolveFinalize() {
    CoordinateSet candidates;
    state.matrix.DFS(GetBotPosition(), candidates);
    if (candidates.count(targetC) == 0) {
        throw StopException();
    }

    if (helper_mode) {
        MoveToCoordinateBFS(targetC, true);
    } else {
        if (!projectionGrounded) {
            if (levitation) {
                AddCommand(Command(Command::Flip));
            }
        }
        MoveToCoordinateBFS(targetC, true);
        AddCommand(Command(Command::Halt));
    }
}

void ReassemblySolverLayersBase::Solve(Trace& output) {
    SolveInit();

    output.tag = "reassembly";

    size_t count = 0;
    for (int x = 0; x < source.GetR(); ++x) {
        for (int y = 0; y < source.GetR(); ++y) {
            for (int z = 0; z < source.GetR(); ++z) {
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
