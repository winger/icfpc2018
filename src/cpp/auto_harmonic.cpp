#include "auto_harmonic.h"

#include "state.h"

struct TimePos
{
    size_t trace_pos;
    size_t bots;
    bool harmonic;
    bool grounded;
};

void ImproveTrace(const Matrix& source, const Matrix& target, const Trace& trace, Trace& output)
{
    output.commands.clear();
    vector<Command> step_commands;
    State s;
    s.Init(source, trace);
    vector<TimePos> vtp;
    vtp.push_back(TimePos{0, 0, 0, 1});
    bool harmonic_used = false;
    for (; s.trace_pos != trace.size(); s.Step())
    {
        vtp.push_back(TimePos{s.trace_pos, s.active_bots.size(), s.harmonics, s.IsGrounded()});
        if (s.harmonics)
            harmonic_used = true;
    }
    if (!harmonic_used) return;
    bool current_harmonic = false;
    for (unsigned i = 1; i < vtp.size(); ++i)
    {
        bool drop_flip = false;
        bool enable_flip = false;
        if (vtp[i].harmonic)
        {
            if (vtp[i].grounded)
            {
                if (vtp[i-1].harmonic)
                {
                    // Everything is fine
                }
                else
                {
                    assert(!current_harmonic);
                    if (vtp[i+1].grounded)
                    {
                        // Can postpone harmonic
                        drop_flip = true;
                    }
                    else
                    {
                        // Everything is fine, need to enable it anyway
                    }
                }
            }
            else
            {
                if (vtp[i-1].harmonic)
                {
                    if (current_harmonic)
                    {
                        // Everything is fine
                    }
                    else
                    {
                        // Need to switch harmonic
                        enable_flip = true;
                    }
                }
                else
                {
                    assert(!current_harmonic);
                }
            }
        }
        else
        {
            assert(vtp[i].grounded);
            if (vtp[i-1].harmonic)
            {
                // Harmonic disabled
                if (current_harmonic)
                {
                    // We need to disable as well
                }
                else
                {
                    drop_flip = true;
                }
            }
            else
            {
                assert(!current_harmonic);
            }
        }

        step_commands.clear();
        bool all_waits = true;
        for (size_t i = vtp[i].trace_pos; i < vtp[i].trace_pos + vtp[i].bots; ++i)
        {
            Command c = trace.commands[i];
            if (c.type == Command::Flip)
            {
                if (drop_flip)
                {
                    c.type = Command::Wait;
                    drop_flip = true;
                }
                else
                {
                    current_harmonic = !current_harmonic;
                }
            }
            if (c.type == Command::Wait)
            {
                if (enable_flip)
                {
                    c.type = Command::Flip;
                    enable_flip = false;
                    current_harmonic = !current_harmonic;
                }
            }
            else
            {
                all_waits = false;
            }
            step_commands.push_back(c);
        }
        if (enable_flip)
        {
            assert(vtp[i].bots > 0);
            Command c(Command::Flip);
            output.commands.push_back(c);
            c.type = Command::Wait;
            for (size_t i = 1; i < vtp[i].bots; ++i)
                output.commands.push_back(c);
            current_harmonic = !current_harmonic;
            enable_flip = false;
        }
        if (!all_waits)
            output.commands.insert(output.commands.end(), step_commands.begin(), step_commands.end());
        assert(!current_harmonic || vtp[i].harmonic);
    }
}
