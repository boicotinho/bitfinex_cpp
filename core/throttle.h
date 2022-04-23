#pragma once
#include "fast_clock.h"
#include "time_util.h"
#include "gcc_utils.h"
#include "str_view.h"
#include <stddef.h>

class Throttle
{
public:
    class Limit;
    explicit Throttle(Limit);

    using Clock = CpuClock;

    bool can_send(Clock::time_point const a_now = Clock::now())
    {
        if(UNLIKELY(a_now < m_next_send))
            return false;
        m_next_send = a_now + m_msg_interval;
        return true;
    }

public:
    class Limit
    {
    public:
        Limit(size_t max_msgs, Nanos wnd);
        explicit Limit(StrView text);

        Limit() = default;
        Limit(Limit const&) = default;
        Limit& operator=(Limit const&) = default;
    private:
        Nanos   m_time_window   {1_s};
        size_t  m_message_limit {10};
    };

private:
    // could queue the send times for a more tight rolling window,
    // but would uses more data.
    Clock::time_point m_next_send {};
    Clock::duration   m_msg_interval {};
};
