#pragma once
//#include "core/vector_view.h"
//#include "core/str_view.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <stdint.h>
#include <stddef.h>

// https://pybind11.readthedocs.io/en/stable/advanced/cast/strings.html
using PyString  = std::string; // TODO: cheapest way to pass string to python?

using Price     = float;
using Quantity  = float;
using ClientOID = uint64_t;
using ClidArray = std::vector<ClientOID>; // VectorView< ClientOID >

enum eSide
{
    BID = 0,
    ASK = 1
};

struct SymbolRef
{
    PyString symbol_name;
    PyString exchange_name;
};

using MarkerCallback = void (*)
    ( SymbolRef const&  // so that python knows which market to issue the cancel
    , ClientOID         // order to cancel
    , Price             // new price that triggered marker
    , Quantity          // quantity at top of book, when triggerd
    , ClidArray const&  // all triggered orders, more aggressive
    );                  // & urgent ones first in the array


// Since it will be very rare that a received data will be truncated,
// throwing exceptions when that happens is OK and simplifies the logic;
struct TruncatedData : std::runtime_error
{
    TruncatedData() : std::runtime_error("TruncatedData") {}
};

struct ProtocolError : std::runtime_error
{
    explicit ProtocolError(std::string const& context = {})
        : std::runtime_error("ProtocolError: " + context) {}
};
