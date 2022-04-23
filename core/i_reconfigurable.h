#pragma once
#include "config.h"

// PLACEHOLDER
// Allow componets to re-read configuration file at run-time, like clickhouse?
struct IReconfigurable
{
    virtual ~IReconfigurable() {}
    virtual void configure(Config const&, std::string const& section={}) {}
    virtual bool reconfigure(Config const& new_cfg) noexcept {return false;}
};
