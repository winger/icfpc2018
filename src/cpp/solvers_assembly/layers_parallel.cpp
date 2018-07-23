#include "layers_parallel.h"

#include "layers_base.h"

#include "../constants.h"
#include "../coordinate_split.h"

static const size_t max_time_for_search = 60000; // in ms

AssemblySolverLayersParallel::AssemblySolverLayersParallel(const Matrix& target, int _split_axis, const vector<int>& _split_coordinate, bool l) {
    matrix = target;
    split_axis = _split_axis;
    split_coordinate = _split_coordinate;
    levitation = l;
}

void AssemblySolverLayersParallel::BuildBot(size_t time, unsigned index, const vector<Trace>& main_traces)
{
    BotTrace& bc = bot_traces[index];
    bc.built_time = time;
    int current_pos = (index == 0) ? 0 : split_coordinate[index - 1] + 1;
    int target_pos = split_coordinate[index];
    for (; current_pos != target_pos; )
    {
        int dp = max(-15, min(target_pos - current_pos, 15));
        Command m(Command::SMove);
        m.cd1 = {(split_axis == 1) ? dp : 0, 0, (split_axis == 3) ? dp : 0};
        bc.trace.commands.push_back(m);
        current_pos += dp;
    }
    if (index + 1 < main_traces.size())
    {
        Command m(Command::Fission);
        m.cd1 = {(split_axis == 1) ? 1 : 0, 0, (split_axis == 3) ? 1 : 0};
        m.m = TaskConsts::N_BOTS - 2 - index;
        bc.trace.commands.push_back(m);
        BuildBot(time + bc.trace.size(), index + 1, main_traces);
    }
    bc.trace.commands.insert(bc.trace.commands.end(), main_traces[index].commands.begin(), main_traces[index].commands.end());
}

void AssemblySolverLayersParallel::MergeBot(unsigned index)
{
    if (index == 0) return;
    BotTrace& bc0 = bot_traces[index - 1];
    BotTrace& bc1 = bot_traces[index];
    int current_pos = split_coordinate[index];
    int target_pos = (index == 0) ? 0 : split_coordinate[index - 1] + 1;
    for (; current_pos != target_pos; )
    {
        int dp = max(-15, min(target_pos - current_pos, 15));
        Command m(Command::SMove);
        m.cd1 = {(split_axis == 1) ? dp : 0, 0, (split_axis == 3) ? dp : 0};
        bc1.trace.commands.push_back(m);
        current_pos += dp;
    }
    size_t time0 = bc0.GetTime();
    size_t time1 = bc1.GetTime();
    size_t time = max(time0, time1);
    bc0.SkipTime(time - time0);
    bc1.SkipTime(time - time1);
    Command m0(Command::FusionP);
    m0.cd1 = {(split_axis == 1) ? 1 : 0, 0, (split_axis == 3) ? 1 : 0};
    bc0.trace.commands.push_back(m0);
    Command m1(Command::FusionS);
    m1.cd1 = {(split_axis == 1) ? -1 : 0, 0, (split_axis == 3) ? -1 : 0};
    bc1.trace.commands.push_back(m1);
    MergeBot(index - 1);
}

void AssemblySolverLayersParallel::Solve(Trace& output)
{
    output.commands.resize(0);
    output.commands.push_back(Command(Command::Flip));

    int r = matrix.GetR();
    assert((split_axis == 1) || (split_axis == 3));
    assert((2 <= split_coordinate.size()) && (split_coordinate.size() <= TaskConsts::N_BOTS + 1));
    assert((split_coordinate[0] == 0) && (split_coordinate.back() == r));

    // Get bots traces
    vector<Trace> personal_traces(split_coordinate.size() - 1);
    Matrix mtemp; mtemp.Init(r);
    for (unsigned i = 0; i < personal_traces.size(); ++i)
    {
        int x0 = (split_axis == 1) ? split_coordinate[i] : 0;
        int x1 = (split_axis == 1) ? split_coordinate[i + 1] : r;
        int z0 = (split_axis == 3) ? split_coordinate[i] : 0;
        int z1 = (split_axis == 3) ? split_coordinate[i + 1] : r;
        mtemp.CopyBlock(matrix, x0, x1, 0, r, z0, z1);
        mtemp.CacheYSlices();
        AssemblySolverLayersBase::SolveHelper(mtemp, {(split_axis == 1) ? split_coordinate[i] : 0, 0, (split_axis == 3) ? split_coordinate[i] : 0}, personal_traces[i], levitation);
        mtemp.EraseBlock(x0, x1, 0, r, z0, z1);
    }

    bot_traces.resize(personal_traces.size());
    BuildBot(0, 0, personal_traces);
    MergeBot(personal_traces.size() - 1);

    size_t last_fill = 0;
    for (const BotTrace& bt : bot_traces)
    {
        for (unsigned i = 0; i < bt.trace.size(); ++i)
        {
            if (bt.trace.commands[i].type == Command::Fill)
                last_fill = max(last_fill, bt.built_time + i + 1);
        }
    }

    bool flip_required = true;
    for(size_t time = 0; time < bot_traces[0].GetTime(); ++time)
    {
        for (const BotTrace& bt : bot_traces)
        {
            if ((time >= bt.built_time) && (time < bt.GetTime()))
            {
                const Command& m = bt.trace.commands[time - bt.built_time];
                if (flip_required && (time > last_fill) && (m.type == Command::Wait))
                {
                    output.commands.push_back(Command(Command::Flip));
                    flip_required = false;
                }
                else
                    output.commands.push_back(m);
            }
        }
    }

    if (flip_required)
        output.commands.push_back(Command(Command::Flip));
    output.commands.push_back(Command(Command::Halt));
    // cout << "Total moves: " << bot_traces[0].GetTime() << endl;
}

void AssemblySolverLayersParallel::Solve(const Matrix& target, int split_axis, const vector<int>& split_coordinate, Trace& output, bool levitation)
{
    AssemblySolverLayersParallel solver(target, split_axis, split_coordinate, levitation);
    solver.Solve(output);
}

Evaluation::Result AssemblySolverLayersParallel::Solve(const Matrix& target, Trace& output, SplitSearchMode mode, bool levitation)
{
    int r = target.GetR();
    int max_bots = min(r, TaskConsts::N_BOTS);
    Trace temp;
    vector<Trace> traces;

    if (mode == base)
    {
        for (int axis = 1; axis <= 3; axis += 2)
        {
            Solve(target, axis, CoordinateSplit::SplitUniform(r, max_bots), temp, levitation);
            traces.push_back(temp);
            Solve(target, axis, CoordinateSplit::SplitByVolume(target, axis, max_bots), temp, levitation);
            traces.push_back(temp);
        }
    }
    else if (mode == base_and_bots)
    {
        for (int bots = 1; bots < max_bots; ++bots)
        {
            for (int axis = 1; axis <= 3; axis += 2)
            {
                Solve(target, axis, CoordinateSplit::SplitUniform(r, bots), temp, levitation);
                traces.push_back(temp);
                Solve(target, axis, CoordinateSplit::SplitByVolume(target, axis, bots), temp, levitation);
                traces.push_back(temp);
            }
        }
    }

    Matrix source(r);
    Evaluation::Result best_result;
    for (const Trace& trace : traces)
    {
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        if (result < best_result)
        {
            best_result = result;
            output = trace;
        }
    }
    return best_result;
}
