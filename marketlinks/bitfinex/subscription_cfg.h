#pragma once
#include "types.h"
#include <string>

namespace bitfinex
{

struct SubscriptionConfig
{
    std::string symbol    {"tBTCUSD"};
    std::string precision {"P0"};  // *P0,...,P4 price aggregation. P0 is 5 significant digits, P4 is just 1 digit
    std::string length    {"25"};  // 1, *25, 100, 250
    std::string freq      {"f0"};  // *f0 (real time), f1 (2 seconds)

    std::string as_json_rpc_request(std::string subId) const;
};

} // namespace bitfinex
