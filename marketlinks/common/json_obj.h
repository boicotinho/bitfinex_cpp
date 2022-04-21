#pragma once
#include <string>
#include <stdexcept>
#include <map>

// A slow but simple and minimal representation of
// JSON objects given their JSON string representation.
// Just enough to parse the incoming market data events (except order book entries)
// https://www.w3schools.com/js/js_json_objects.asp
class JsonObj
{
    std::map<std::string, std::string> m_key_value_map;
public:

    JsonObj() = default;
    JsonObj(char const* const bgn, char const* const end);
    explicit JsonObj(const std::string& str)
        : JsonObj(str.c_str(), str.c_str() + str.length()) {}

    bool try_get(std::string const& key, std::string& out_val) const;

    std::string get(std::string const& key) const; // throws ParseError

    struct ParseError : public std::runtime_error
        {
        explicit ParseError(std::string const& details="");
        };
};
