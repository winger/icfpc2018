#pragma once

#include "../evaluation.h"
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

    int split_axis; // 1 - x, 3 - z
    vector<int> split_coordinate;
    vector<BotTrace> bot_traces;

    AssemblySolverLayersParallel(const Matrix& target, int _split_axis, const vector<int>& _split_coordinate);

    void BuildBot(size_t time, unsigned index, const vector<Trace>& main_traces);
    void MergeBot(unsigned index);

    // TODO:
    //   Duplicate old function somethere. It useful for small tests.
    // void FindBestSplit();
    void Solve(Trace& output);

public:
    static void Solve(const Matrix& target, int split_axis, const vector<int>& split_coordinate, Trace& output);


    enum SplitSearchMode
    {
        base,
        base_and_bots
    };

    static Evaluation::Result Solve(const Matrix& target, Trace& output, SplitSearchMode mode = base);
};
