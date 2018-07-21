#include "layers_parallel.h"

#include "layers_base.h"

SolverLayersParallel::SolverLayersParallel(const Matrix& m)
{
    matrix = m;
}

void SolverLayersParallel::BuildBot(size_t time, unsigned index, const vector<Trace>& main_traces)
{
    BotTrace& bc = bot_traces[index];
    bc.built_time = time;
    int current_pos = (index == 0) ? 0 : split_coordinate[index - 1] + 1;
    int target_pos = split_coordinate[index];
    // cout << "BuildBot: " << index << "/" << main_traces.size() << " " << current_pos << " " << target_pos << " " << matrix.GetR() << endl;
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
        m.m = 18 - index;
        bc.trace.commands.push_back(m);
        BuildBot(time + bc.trace.size(), index + 1, main_traces);
    }
    bc.trace.commands.insert(bc.trace.commands.end(), main_traces[index].commands.begin(), main_traces[index].commands.end());
}

void SolverLayersParallel::MergeBot(unsigned index)
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

void SolverLayersParallel::SolverLayersParallel::FindBestSplit()
{
    split_axis = 1;
    int r = matrix.GetR();
    split_coordinate.resize(0);
    for (unsigned i = 0; i <= 20; ++i)
        split_coordinate.push_back((i * r) / 20);        
}

void SolverLayersParallel::Solve(Trace& output)
{
    output.commands.resize(0);
    output.commands.push_back(Command(Command::Flip));

    FindBestSplit();
    int r = matrix.GetR();
    assert((split_axis == 1) || (split_axis == 3));
    assert((2 <= split_coordinate.size()) && (split_coordinate.size() <= 21));
    assert((split_coordinate[0] == 0) && (split_coordinate.back() == r));

    assert(split_axis == 1); // We will support split_axis == 3 later.

    // Get bots traces
    vector<Trace> personal_traces(split_coordinate.size() - 1);
    Matrix mtemp; mtemp.Init(r);
    for (unsigned i = 0; i < personal_traces.size(); ++i)
    {
        int x0 = (split_axis == 1) ? split_coordinate[i] : 0;
        int x1 = (split_axis == 1) ? split_coordinate[i + 1] : r;
        int z0 = (split_axis == 3) ? split_coordinate[i] : 0;
        int z1 = (split_axis == 3) ? split_coordinate[i + 1] : r;
        for (int x = x0; x < x1; ++x)
        {
            for (int y = 0; y < r; ++y)
            {
                for (int z = z0; z < z1; ++z)
                {
                    if (matrix.Get(x, y, z))
                        mtemp.Fill(x, y, z);
                }
            }
        }
        SolverLayersBase::SolveHelper(mtemp, {split_coordinate[i], 0, 0}, personal_traces[i]);
        for (int x = x0; x < x1; ++x)
        {
            for (int y = 0; y < r; ++y)
            {
                for (int z = z0; z < z1; ++z)
                {
                    mtemp.Erase(x, y, z);
                }
            }
        }
    }

    bot_traces.resize(personal_traces.size());
    BuildBot(0, 0, personal_traces);
    MergeBot(personal_traces.size() - 1);

    for(size_t time = 0; time < bot_traces[0].GetTime(); ++time)
    {
        for (const BotTrace& bt : bot_traces)
        {
            if ((time >= bt.built_time) && (time < bt.GetTime()))
            {
                output.commands.push_back(bt.trace.commands[time - bt.built_time]);
            }
        }
    }

    output.commands.push_back(Command(Command::Flip));
    output.commands.push_back(Command(Command::Halt));
}

uint64_t SolverLayersParallel::Solve(const Matrix& m, Trace& output)
{
    SolverLayersParallel solver(m);
    solver.Solve(output);
    return 0;
}
