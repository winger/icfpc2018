#include "layers_parallel.h"

#include "layers_base.h"

#include "../timer.h"
#include "../constants.h"

static const size_t max_time_for_search = 60000; // in ms

AssemblySolverLayersParallel::AssemblySolverLayersParallel(const Matrix& m, bool _search_best_split)
{
    matrix = m;
    search_best_split = _search_best_split;
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
        m.m = 18 - index;
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

void AssemblySolverLayersParallel::FindBestSplit()
{
    struct SplitInfo
    {
        bool valid = false;
        size_t moves;
        uint64_t energy;
        vector<int> splits;
    };

    int r = matrix.GetR();
    split_axis = 1;
    split_coordinate.resize(0);
    for (unsigned i = 0; i <= TaskConsts::N_BOTS; ++i)
        split_coordinate.push_back((i * r) / TaskConsts::N_BOTS);
    if (!search_best_split) return;

    Matrix mtemp; mtemp.Init(r);
    bool best_split_valid = false;
    size_t best_moves = 0, best_energy = 0;

    for (int axis = 1; axis <= 3; axis += 2)
    {
        Timer t;
        // don't check all positions because too expensive
        vector<size_t> volume(r, 0);
        size_t total_volume = 0;
        for (int x = 0; x < r; ++x)
        {
            for (int y = 0; y < r; ++y)
            {
                for (int z = 0; z < r; ++z)
                {
                    if (matrix.Get(x, y, z))
                    {
                        volume[(axis == 1) ? x : z] += 1;
                        total_volume += 1;
                    }
                }
            }
        }
        vector<size_t> volume2(r + 1, 0);
        volume2[0] = volume2[r] = total_volume;
        size_t last_volume = volume[0];
        for (int i = 1; i < r; ++i)
        {
            if (volume[i] > 0)
            {
                volume2[i] = volume[i] + last_volume;
                last_volume = volume[i];
            }
        }

        vector<pair<size_t, int>> vp;
        for (int i = 0; i <= r; ++i)
        {
            if (volume2[i] > 0)
            {
                vp.push_back({volume2[i], i});
            }
        }
        sort(vp.begin(), vp.end());
        reverse(vp.begin(), vp.end());

        Trace trace;
        vector<vector<pair<size_t, uint64_t>>> vcost(r, vector<pair<size_t, uint64_t>>(r + 1, {0, 0}));
        for (unsigned i0 = 1; i0 < vp.size(); ++i0)
        {
            for (unsigned j0 = 0; j0 < i0; ++j0)
            {
                int i = min(vp[i0].second, vp[j0].second);
                int j = max(vp[i0].second, vp[j0].second);
                int x0 = (axis == 1) ? i : 0;
                int x1 = (axis == 1) ? j : r;
                int z0 = (axis == 3) ? i : 0;
                int z1 = (axis == 3) ? j : r;
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
                uint64_t energy = AssemblySolverLayersBase::SolveHelper(mtemp, {(axis == 1) ? i : 0, 0, (axis == 3) ? i : 0}, trace).energy;
                size_t moves = trace.size();
                energy -= 30 * matrix.GetVolume() * moves; // We will pay for moves later
                energy -= 20 * moves; // We will pay for bot-moves later
                vcost[i][j] = {moves, energy};
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

                if (t.GetMilliseconds() > max_time_for_search / 2)
                    break;
            }
            if (t.GetMilliseconds() > max_time_for_search / 2)
                break;
        }

        vector<vector<SplitInfo>> vdp(TaskConsts::N_BOTS, vector<SplitInfo>(r+1));
        for (int k = 1; k <= r; ++k)
        {
            if (vcost[0][k].first == 0) continue;
            vdp[0][k].valid = true;
            vdp[0][k].moves = vcost[0][k].first;
            vdp[0][k].energy = vcost[0][k].second;
            vdp[0][k].splits = {0, k};
        }
        for (unsigned l = 1; l < TaskConsts::N_BOTS; ++l)
        {
            for (int m = l; m < r; ++m)
            {
                if (!vdp[l-1][m].valid) continue;
                for (int k = m + 1; k <= r; ++k)
                {
                    size_t new_moves = vcost[m][k].first;
                    if (new_moves == 0) continue;
                    size_t extra_moves = 4 * l + 2 * (m / 15);
                    size_t total_moves = max(vdp[l-1][m].moves, new_moves + extra_moves);
                    size_t energy = vdp[l-1][m].energy + vcost[m][k].second;
                    if (!vdp[l][k].valid || (vdp[l][k].moves > total_moves) || ((vdp[l][k].moves == total_moves) && (vdp[l][k].energy > energy)))
                    {
                        vdp[l][k].valid = true;
                        vdp[l][k].moves = total_moves;
                        vdp[l][k].energy = energy;
                        vdp[l][k].splits = vdp[l-1][m].splits;
                        vdp[l][k].splits.push_back(k);
                    }
                }
            }
        }
        for (unsigned l = 0; l < TaskConsts::N_BOTS; ++l)
        {
            if (!vdp[l][r].valid)
                continue;
            size_t energy = vdp[l][r].energy + 20 * l * vdp[l][r].moves; // penaly for bots
            if (!best_split_valid || (best_moves > vdp[l][r].moves) || ((best_moves == vdp[l][r].moves) && (best_energy > energy)))
            {
                best_split_valid = true;
                best_moves = vdp[l][r].moves;
                best_energy = energy;
                split_axis = axis;
                split_coordinate = vdp[l][r].splits;
            }
        }
    }
    // cout << "Split: " << split_axis << " " << split_coordinate.size() << " " << best_moves << endl;
    // assert(best_split_valid);
}

void AssemblySolverLayersParallel::Solve(Trace& output)
{
    output.commands.resize(0);
    output.commands.push_back(Command(Command::Flip));

    FindBestSplit();
    int r = matrix.GetR();
    assert((split_axis == 1) || (split_axis == 3));
    assert((2 <= split_coordinate.size()) && (split_coordinate.size() <= 21));
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
        AssemblySolverLayersBase::SolveHelper(mtemp, {(split_axis == 1) ? split_coordinate[i] : 0, 0, (split_axis == 3) ? split_coordinate[i] : 0}, personal_traces[i]);
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

uint64_t AssemblySolverLayersParallel::Solve(const Matrix& m, Trace& output, bool search_best_split)
{
    AssemblySolverLayersParallel solver(m, search_best_split);
    solver.Solve(output);
    return 0;
}
