#include "json_obj.h"
#include <sstream>

// This needs boost version 1.75, not sure if we want to bump the version that high up...
// Maybe should write own parser instead?
#include <boost/json.hpp>
#include <boost/json/src.hpp>
using namespace boost::json;

namespace bitfinex
{

JsonObj::ParseError::ParseError(std::string const& details)
    : std::runtime_error("JsonObj ParseError error; " + details)
{
}

// Input Exaples:
// { "event": "conf", "flags": "131072" }
// { "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" }
// {"event":"conf","status":"OK"}
// {"event":"info","version":2,"serverId":"ba599674-2ff9-43a0-bb0a-eb3174ab2073","platform":{"status":1}}
// {"event":"subscribed","channel":"book","chanId":266343,"symbol":"tBTCUSD","prec":"P0","freq":"F0","len":"25","pair":"BTCUSD"}
JsonObj::JsonObj(char const* const bgn, char const* const end)
{
    std::string input(std::string(bgn, end));
    try
    {
        value jv = parse(input);
        object& obj = jv.as_object();
        for(const auto& kvp: obj)
        {
            auto const& key = kvp.key_c_str();
            auto const& val = kvp.value();
            auto const kind = val.kind();
            std::stringstream ss_val;
            ss_val << val;
            std::string vv = ss_val.str();
            if(boost::json::kind::string == kind)
                vv = vv.substr(1,vv.size()-2);
            m_key_value_map.insert({key, vv});
        }
    }
    catch(const std::exception& ex)
    {
        throw ParseError(ex.what() + (": " + input));
    }
}

bool JsonObj::try_get(std::string const& key, std::string& out_val) const
{
    auto it = m_key_value_map.find(key);
    if(it == m_key_value_map.end())
        return false;
    out_val = it->second;
    return true;
}

std::string JsonObj::get(std::string const& key) const
{
    std::string val;
    if(!try_get(key, val))
        throw ParseError("Key not found: '" + key + "'");
    return val;
}

} // namespace bitfinex
