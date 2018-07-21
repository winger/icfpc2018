#include "state.h"

void State::Init(int r, const Trace& _trace)
{
    correct = true;
    energy = 0;
    harmonics = 0;
    matrix.Init(r);
    all_bots.resize(0);
    all_bots.resize(20);
    for (unsigned i = 0; i < 20; ++i)
    {
        all_bots[i].bid = i;
        if (i)
            all_bots[0].seeds.push_back(i);
    }
    all_bots[0].c = {0, 0, 0};
    active_bots.push_back(0);
    trace = _trace;
    trace_pos = 0;
}

bool State::IsCorrectFinal() const
{
    return correct && (harmonics == 0) && (active_bots.size() == 0) && (trace_pos == trace.size());
}

void State::Step()
{
    vector<unsigned> bots = active_bots;
    assert(trace_pos + bots.size() <= trace.size());
    energy += (harmonics ? 30 : 3) * matrix.GetVolume();
    energy += 20 * bots.size();
    InterfereCheck ic;
    for (unsigned bid : bots)
    {
        BotState& bs = all_bots[bid];
        ic.AddCoordinate(bs.c);
        Command c = trace.commands[trace_pos++];
        if (c.type == Command::Halt)
        {
            correct = correct && (bs.c.x == 0) && (bs.c.y == 0) && (bs.c.z == 0) && (bots.size() == 1) && (harmonics == false) && (trace_pos == trace.size());
            assert(correct);
            active_bots.clear();
        }
        else if (c.type == Command::Wait)
        {            
        }
        else if (c.type == Command::Flip)
        {
            harmonics = !harmonics;
        }
        else if (c.type == Command::SMove)
        {           
            correct = correct && c.cd1.IsLongLinearCoordinateDifferences() && MoveBot(bs, ic, c.cd1);
            assert(correct);
        }
        else if (c.type == Command::LMove)
        {
            correct = correct && c.cd1.IsShortLinearCoordinateDifferences() && MoveBot(bs, ic, c.cd1);
            correct = correct && c.cd2.IsShortLinearCoordinateDifferences() && MoveBot(bs, ic, c.cd2);
            assert(correct);
            energy += 4;
        }
        else if (c.type == Command::Fission)
        {
            Coordinate fc = bs.c + c.cd1;
            correct = correct && matrix.IsInside(fc) && !matrix.Get(fc) && (c.m + 1 <= bs.seeds.size());
            assert(correct);
            active_bots.push_back(bs.seeds[0]);
            sort(active_bots.begin(), active_bots.end());
            BotState& jbs = all_bots[bs.seeds[0]];
            jbs.c = fc;
            jbs.seeds = vector<unsigned> {bs.seeds.begin() + 1, bs.seeds.begin() + 1 + c.m};
            bs.seeds.erase(bs.seeds.begin(), bs.seeds.begin() + 1 + c.m);
            energy += 24;
        }
        else if (c.type == Command::Fill)
        {
            Coordinate fc = bs.c + c.cd1;
            correct = correct && matrix.IsInside(fc);
            assert(correct);
            energy += matrix.Get(fc) ? 6 : 12;
            matrix.Fill(fc);
            ic.AddCoordinate(fc);
        }
        else if (c.type == Command::FusionP)
        {
            Coordinate pc = bs.c + c.cd1;
            unsigned bid2 = bid;
            for (unsigned jbid : bots)
            {
                BotState& jbs = all_bots[jbid];
                if (jbs.c == pc)
                {
                    // TODO:
                    //   Check command on the second bot
                    bid2 = jbid;
                    break;
                }
            }
            correct = correct && (bid2 != bid);
            assert(correct);
            bs.seeds.push_back(bid2);
            bs.seeds.insert(bs.seeds.end(), all_bots[bid2].seeds.begin(), all_bots[bid2].seeds.end());
            sort(bs.seeds.begin(), bs.seeds.end());
        }
        else if (c.type == Command::FusionS)
        {
            Coordinate pc = bs.c + c.cd1;
            bool found = false;
            for (unsigned jbid : bots)
            {
                BotState& jbs = all_bots[jbid];
                if (jbs.c == pc)
                {
                    // TODO:
                    //   Check command on the second bot
                    found = true;
                    break;
                }
            }
            correct = correct && found;
            assert(correct);
            auto it = find(active_bots.begin(), active_bots.end(), bid);
            assert(it != active_bots.end());
            active_bots.erase(it);
            energy -= 24;
        }
    }
    correct = correct && ic.IsValid();
    assert(correct);
    if (!correct)
        trace_pos = trace.size();
}

void State::Run()
{
    for (; trace_pos < trace.size(); )
        Step();
}

bool State::MoveBot(BotState& bs, InterfereCheck& ic, const CoordinateDifference& cd)
{
    assert(cd.IsLinearCoordinateDifferences());
    CoordinateDifference step { sign(cd.dx), sign(cd.dy), sign(cd.dz) };
    unsigned l = cd.ManhattanLength();
    for (unsigned i = 1; i <= l; ++i)
    {
        bs.c = bs.c + step;
        correct = correct && matrix.IsInside(bs.c) && !matrix.Get(bs.c);
        ic.AddCoordinate(bs.c);
        energy += 2;
    }
    return correct;
}
