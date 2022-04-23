#pragma once
#include <string>
#include <chrono>
#include <type_traits>
#include <stdexcept>
#include <iosfwd>
#include <stdint.h>

template<class TVal>
using EnableIfIntegral =
    typename std::enable_if< std::is_integral<TVal>::value >::type*;

class Config
{
public:
    using key_t = std::string const&;
    struct Error : std::runtime_error { Error(key_t); };

    void load(std::istream&);
    void save(std::ostream&);

    template<class TVal>
    TVal get(key_t a_key)
        {
        TVal res;
        if(!try_get(a_key, res))
            throw Error(a_key);
        return res;
        }

    bool try_get(key_t, std::string&) const noexcept;
    bool try_get(key_t, int64_t&) const noexcept;
    bool try_get(key_t, double&) const noexcept;
    bool try_get(key_t, float&) const noexcept;
    bool try_get(key_t, bool&) const noexcept;
    bool try_get(key_t, std::chrono::nanoseconds&) const noexcept;

    template<class TVal>
    bool try_get( key_t a_key
                , TVal& a_val
                , EnableIfIntegral<TVal> = 0) const noexcept
                {
                int64_t tmp;
                if(!try_get(a_key, tmp))
                    return false;
                a_val = tmp;
                return true;
                }

    bool has_key(key_t) const noexcept;

    // SUB-SECTION //

    // Enters sub-section/prefix/json subobject or similar, like using namespace
    Config get_section(std::string const& prefix) const;
    bool try_get_section(std::string const& prefix, Config const&);

    // WRITE //

    void set(key_t, std::string const&) noexcept;
    void set(key_t, int64_t) noexcept;
    void set(key_t, double) noexcept;
    void set(key_t, float) noexcept;
    void set(key_t, bool) noexcept;
    void set(key_t, std::chrono::nanoseconds) noexcept;

    template<class TVal>
    void set(key_t a_key, TVal a_val, EnableIfIntegral<TVal> = 0) noexcept
        { int64_t val = a_val; set(a_key, val); }
private:
};
