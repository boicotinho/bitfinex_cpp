#include "json_obj.h"
#include <sstream>
#include <iostream>
#include <ctype.h>

#ifndef USE_BOOST_JSON
    #define USE_BOOST_JSON 0
#endif

#if (USE_BOOST_JSON)

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

// Input Examples:
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

#else // USE_BOOST_JSON

// Very simple dependency-free JSON parser, enough for Bitfinex streams
namespace bitfinex
{

enum class eEvent : uint8_t {
    CHAR,           // alphanumeric character
    QUOTE,          // "
    COLON_SEP,      // :
    COMMA,          // ,
    WSPC,           // white space
    CBRKT_OPEN,     // {
    CBRKT_CLOSE,    // }
    SBRKT_OPEN,     // [
    SBRKT_CLOSE,    // ]
    COUNT
};

// SC: Scanning/Searching for something, looking for its beginning.
// RD: Reading something. Already started consuming its characters.
enum class eState : uint8_t {
    SC_ROOT,        // scanning for root {    
    SC_KEY,         // scanning for key " start.may find blanks too
    RD_KEY,         // reading key until "
    SC_COLON,       // scanning for :  may find blanks too
    SC_VAL_BGN,     // scanning for beginning " of value. may also }
    RD_VAL_QUO,     // quoted string, "status":"OK" , will stop at the ending quote
    RD_VAL_UNQ,     // unquoted single value, "chanId" : 266343, "Pi":3.14, "car":null, "sale":true, "qwerty":false
    RD_VAL_OBJ,     // reading key:value pair when value is an embedded object ; e.g. "platform" : {"status":1},
    RD_VAL_ARY,     // "cars" : ["Ford", "BMW", "Fiat"]
    SC_OBJ_END,     // may find , }
    DONT_CHANGE_STATE, COUNT = DONT_CHANGE_STATE
};

enum class eAction : uint8_t {
    IGNORE,         // ignore current char
    PTR_KEY,        // point curr_token_ptr = &key,
    PTR_VAL,        // point curr_token_ptr = &val, always just std::string
    PUSH,           // push character to *curr_token_ptr : {&key, &val}
    INSERTN,        // insert (key,value) pair into the resulting map, not including current char
    INSERTI,        // insert (key,value) pair into the resulting map, including current char
    ENT_SUB,        // enter sub-object, currently we don't need to parse sub-objects. pushes a {
    LEV_SUB,        // leave sub-object, pops a }, if all  } are popped, call INSERT then set curr_state = SC_OBJ_END
    REJECT,         // reject the JSON input as error
    ACCEPT,         // accept the JSON input and finish parsing
    ACCEPTI,        // ACCEPT + insert pending (key,pair) into the map
    COUNT
};

struct FsmTransition {
    eAction  action;
    eState   new_state;
};
#define T(action,state) FsmTransition{eAction::action, eState::state}
#define ___$___     REJECT // action for indicating error
#define _______     IGNORE // action for doing nothing besides maybe changing state
#define __________  DONT_CHANGE_STATE
constexpr FsmTransition g_FSM_table [(int)eState::COUNT] [(int)eEvent::COUNT] = {
                 //======================+=======================+=======================+=======================+=======================+=======================+=======================+=======================+=========================//
                 //   alphanum CHAR      |        " QUOTE        |     : COLON_SEP       |       , COMMA         |    white space WSPC   |      { CBRKT_OPEN     |     } CBRKT_CLOSE     |     [ SBRKT_OPEN      |    ] SBRKT_CLOSE        //
                 //======================+=======================+=======================+=======================+=======================+=======================+=======================+=======================+=========================//
/* SC_ROOT    */ { T(___$___, __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), T(_______, __________), T(_______, SC_KEY    ), T(___$___, __________), T(___$___, __________), T(___$___, __________), },
/* SC_KEY     */ { T(___$___, __________), T(PTR_KEY, RD_KEY    ), T(___$___, __________), T(___$___, __________), T(_______, __________), T(___$___, __________), T(ACCEPT , __________), T(___$___, __________), T(___$___, __________), },
/* RD_KEY     */ { T(PUSH   , __________), T(PTR_VAL, SC_COLON  ), T(___$___, __________), T(___$___, __________), T(PUSH   , __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), },
/* SC_COLON   */ { T(___$___, __________), T(___$___, __________), T(_______, SC_VAL_BGN), T(___$___, __________), T(_______, __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), },
/* SC_VAL_BGN */ { T(PUSH   , RD_VAL_UNQ), T(_______, RD_VAL_QUO), T(___$___, __________), T(___$___, __________), T(_______, __________), T(ENT_SUB, RD_VAL_OBJ), T(___$___, __________), T(PUSH   , RD_VAL_ARY), T(___$___, __________), },
/* RD_VAL_QUO */ { T(PUSH   , __________), T(INSERTN, SC_OBJ_END), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), },
/* RD_VAL_UNQ */ { T(PUSH   , __________), T(___$___, __________), T(___$___, __________), T(INSERTN, SC_KEY    ), T(INSERTN, SC_OBJ_END), T(___$___, __________), T(ACCEPTI, __________), T(___$___, __________), T(___$___, __________), },
/* RD_VAL_OBJ */ { T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(PUSH   , __________), T(ENT_SUB, __________), T(LEV_SUB, __________), T(PUSH   , __________), T(PUSH   , __________), },
/* RD_VAL_ARY */ { T(PUSH   , __________), T(PUSH   , __________), T(___$___, __________), T(PUSH   , __________), T(PUSH   , __________), T(___$___, __________), T(___$___, __________), T(___$___, __________), T(INSERTI, SC_OBJ_END), },
/* SC_OBJ_END */ { T(___$___, __________), T(___$___, __________), T(___$___, __________), T(_______, SC_KEY    ), T(_______, __________), T(___$___, __________), T(ACCEPT , SC_ROOT   ), T(___$___, __________), T(___$___, __________), },
};
#undef T
#undef ___$___
#undef __________

// Example of value types which must be accepted:
// { "event" : "xyz" , "num"    : 131072, "Pi",  : 3.14, "car" : null, 
//   "sale"  : true  , "qwerty" : false,  "cars" : ["Ford", "BMW", "Fiat"],
//   "platform" : {"status":1}
// }
JsonObj::JsonObj(char const* const bgn, char const* const end)
{
    eState       curr_state = eState::SC_ROOT;
    std::string  curr_key;
    std::string  curr_val;
    std::string* curr_token_ptr = &curr_key;
    int          sub_obj = 0; // recursion depth when parsing object-values:  "platform":{"status":1}}

    //std::cerr << "\n" << std::string(bgn, end) << "\n";
    auto insert_into_map = [&](char include_char)
    {
        if(include_char)
            curr_val.push_back(include_char);
        m_key_value_map [curr_key] = curr_val;
        //std::cerr << curr_key << " = " << curr_val << "\n";
        curr_key.clear();
        curr_val.clear();
        curr_token_ptr = &curr_key;
    };

    for(auto pp = bgn; pp < end; ++pp)
    {
        eEvent event = eEvent::COUNT;
        switch(*pp)
        {
            case '"' : event = eEvent::QUOTE;       break;
            case ':' : event = eEvent::COLON_SEP;   break;
            case ',' : event = eEvent::COMMA;       break;
            case '\t': 
            case ' ' : event = eEvent::WSPC;        break;
            case '{' : event = eEvent::CBRKT_OPEN;  break;
            case '}' : event = eEvent::CBRKT_CLOSE; break;
            case '[' : event = eEvent::SBRKT_OPEN;  break;
            case ']' : event = eEvent::SBRKT_CLOSE; break;
            default: 
                if(isalnum(*pp) || '.' == *pp ||
                    (eState::RD_VAL_QUO == curr_state && isprint(*pp)))
                {
                    event = eEvent::CHAR;
                    break;
                }
                throw ParseError("Invalid char.");
        }

        const FsmTransition transition = g_FSM_table [(int)curr_state] [(int)event];

        auto const prev_state = curr_state;

        if(transition.new_state != eState::DONT_CHANGE_STATE)
            curr_state = transition.new_state;

        switch(transition.action)
        {
            case eAction::IGNORE    : {                                         } break;
            case eAction::REJECT    : { throw ParseError("Invalid syntax.");    } break;
            case eAction::ACCEPT    : { return;                                 } break;
            case eAction::ACCEPTI   : { insert_into_map(0); return;             } break;
            case eAction::PTR_KEY   : { curr_token_ptr = &curr_key;             } break;
            case eAction::PTR_VAL   : { curr_token_ptr = &curr_val;             } break;
            case eAction::PUSH      : { (*curr_token_ptr) += *pp;               } break;
            case eAction::INSERTI   : { insert_into_map(*pp);                   } break;
            case eAction::INSERTN   : { insert_into_map(0);                     } break;
            case eAction::ENT_SUB   : { (*curr_token_ptr) += *pp;  ++sub_obj;   } break;
            case eAction::LEV_SUB   : { 
                (*curr_token_ptr) += *pp;
                if(0 == --sub_obj) { // recursion when value is an embedded object
                    insert_into_map(0);
                    curr_state = eState::SC_OBJ_END;
                }
            } break;
            default :
                throw ParseError("Invalid action.");
        }

    }
    throw ParseError("Unexpected end of input.");
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

JsonObj::ParseError::ParseError(std::string const& details)
    : std::runtime_error("JsonObj ParseError error; " + details)
{
}

} // namespace bitfinex

#endif // USE_BOOST_JSON
