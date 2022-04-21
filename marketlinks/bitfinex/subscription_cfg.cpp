#include "subscription_cfg.h"
#include "core/string_utils.h"

namespace bitfinex
{

std::string SubscriptionConfig::as_json_rpc_request(std::string subId) const
{
    if(subId.empty())
        return format_string(
            R"({"event": "subscribe", "channel": "book", "symbol": "%s", "prec": "%s", "len": "%s"})",
            symbol.c_str(), precision.c_str(), length.c_str());
    else
        return format_string(
            R"({"event": "subscribe", "channel": "book", "symbol": "%s", "prec": "%s", "len": "%s", "subId":"%s"})",
            symbol.c_str(), precision.c_str(), length.c_str(), subId.c_str());
}

} // namespace bitfinex
