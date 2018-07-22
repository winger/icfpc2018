#pragma once

#include "../solver.h"
#include "../state.h"

// Similar to SolverLayersBase but split Cube to pieces and solve each of them by their own bot.
class AssemblySolverLayersParallel
{
protected:
    struct BotTrace
    {
        size_t built_time;
        Trace trace;

        size_t GetTime() const { return built_time + trace.size(); }
        void SkipTime(size_t time) {
            for (unsigned i = 0; i < time; ++i) {
                trace.commands.push_back(Command(Command::Wait));
            }
        }
    };

    Matrix matrix;

    bool search_best_split;
    int split_axis; // 1 - x, 3 - z
    vector<int> split_coordinate;
    vector<BotTrace> bot_traces;

    AssemblySolverLayersParallel(const Matrix& m, bool search_best_split);

    void BuildBot(size_t time, unsigned index, const vector<Trace>& main_traces);
    void MergeBot(unsigned index);

    void FindBestSplit();
    void Solve(Trace& output);

public:
    static uint64_t Solve(const Matrix& m, Trace& output, bool search_best_split = false);
};
