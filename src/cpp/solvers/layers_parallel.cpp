#include "layers_parallel.h"

#include "layers_base.h"

SolverLayersParallel::SolverLayersParallel(const Matrix& m, bool _search_best_split)
{
    matrix = m;
    search_best_split = _search_best_split;
}

void SolverLayersParallel::BuildBot(size_t time, unsigned index, const vector<Trace>& main_traces)
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
    struct SplitInfo
    {
        bool valid = false;
        size_t moves;
        uint64_t energy;
        vector<int> splits;
    };

    int r = matrix.GetR();
    if (!search_best_split)
    {
        split_axis = 1;
        split_coordinate.resize(0);
        for (unsigned i = 0; i <= 20; ++i)
            split_coordinate.push_back((i * r) / 20);        
        return;
    }

    Matrix mtemp; mtemp.Init(r);
    bool best_split_valid = false;
    size_t best_moves = 0, best_energy = 0;
    
    for (int axis = 1; axis <= 3; axis += 2)
    {
        // don't check all positions because too expensive
        vector<bool> validk(r + 1, false);
        validk[0] = true; validk[r] = true;
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
        size_t current_volume = 0, delta_volume = max(size_t(1), total_volume / 20), next_volume = delta_volume;
        int last_valid_k = 0;
        bool enable_next_k = false;
        for (int k = 0; k < r; ++k)
        {
            if (volume[k] == 0)
                continue;                
            if (enable_next_k || (50 * (k - last_valid_k) > r))
            {
                validk[k] = true;
                last_valid_k = k;
                enable_next_k = false;
            }
            current_volume += volume[k];
            if (current_volume >= next_volume)
            {
                validk[k] = true;
                last_valid_k = k;
                enable_next_k = true;
                for (; next_volume <= current_volume; ) next_volume += delta_volume;
            }
        }


        Trace trace;
        vector<vector<pair<size_t, uint64_t>>> vcost(r, vector<pair<uint64_t, size_t>>(r, {0, 0}));
        for (int i = 0; i < r; ++i)
        {
            if (!validk[i]) continue;
            for (int j = i; j < r; ++j)
            {
                if (!validk[j+1]) continue;
                int x0 = (axis == 1) ? i : 0;
                int x1 = (axis == 1) ? j + 1 : r;
                int z0 = (axis == 3) ? i : 0;
                int z1 = (axis == 3) ? j + 1 : r;
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
                uint64_t energy = SolverLayersBase::SolveHelper(mtemp, {(axis == 1) ? i : 0, 0, (axis == 3) ? i : 0}, trace);
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
            }
        }

        vector<vector<SplitInfo>> vdp(20, vector<SplitInfo>(r));
        for (int k = 0; k < r; ++k)
        {
            if (vcost[0][k].first == 0) continue;
            vdp[0][k].valid = true;
            vdp[0][k].moves = vcost[0][k].first;
            vdp[0][k].energy = vcost[0][k].second;
            vdp[0][k].splits = {0, k + 1};
        }
        for (unsigned l = 1; l < 20; ++l)
        {
            for (int k = l; k < r; ++k)
            {
                if (!validk[k+1]) continue;
                for (int m = l - 1; m < k; ++m)
                {
                    size_t new_moves = vcost[m + 1][k].first;
                    if (new_moves == 0) continue;
                    size_t extra_moves = 4 * l + 2 * (k / 15);
                    size_t total_moves = max(vdp[l-1][m].moves, new_moves + extra_moves);
                    size_t energy = vdp[l-1][m].energy + vcost[m + 1][k].second;
                    if (!vdp[l][k].valid || (vdp[l][k].moves > total_moves) || ((vdp[l][k].moves == total_moves) && (vdp[l][k].energy > energy)))
                    {
                        vdp[l][k].valid = true;
                        vdp[l][k].moves = total_moves;
                        vdp[l][k].energy = energy;
                        vdp[l][k].splits = vdp[l-1][m].splits;
                        vdp[l][k].splits.push_back(k + 1);
                    }
                }
            }
        }
        for (unsigned l = 0; l < 20; ++l)
        {
            if (!vdp[l][r - 1].valid)
                continue;
            size_t energy = vdp[l][r - 1].energy + 20 * l * vdp[l][r - 1].moves; // penaly for bots
            if (!best_split_valid || (best_moves > vdp[l][r-1].moves) || ((best_moves == vdp[l][r-1].moves) && (best_energy > energy)))
            {
                best_split_valid = true;
                best_moves = vdp[l][r-1].moves;
                best_energy = energy;
                split_axis = axis;
                split_coordinate = vdp[l][r - 1].splits;
            }
        }
    }
    // cout << "Split: " << split_axis << " " << split_coordinate.size() << " " << best_moves << endl;
    assert(best_split_valid);
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
        SolverLayersBase::SolveHelper(mtemp, {(split_axis == 1) ? split_coordinate[i] : 0, 0, (split_axis == 3) ? split_coordinate[i] : 0}, personal_traces[i]);
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
                last_fill = max(last_fill, bt.start_time + i);
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

uint64_t SolverLayersParallel::Solve(const Matrix& m, Trace& output, bool search_best_split)
{
    SolverLayersParallel solver(m, search_best_split);
    solver.Solve(output);
    return 0;
}
