#pragma once
#include <string>
#include <vector>
#include "core/any.h"

// Placeholder
struct IStatsSource
{
    virtual ~IStatsSource() {}
    using StatName = std::string;

    virtual std::vector<StatName>
        stats_enum() const { return {}; }

    virtual void
        stats_read_thread_safe(std::vector<Any>& out) const {}
};
