#pragma once
#include "gcc_utils.h"
#include "color.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#define TLOG_INF(FMT,...) fprintf(stderr,"%sMemSocket INF%s " FMT "\n", COL_BLUE,   COL_RESET, __VA_ARGS__)
#define TLOG_WAR(FMT,...) fprintf(stderr,"%sMemSocket WAR%s " FMT "\n", COL_YELLOW, COL_RESET, __VA_ARGS__)
#define TLOG_ERR(FMT,...) fprintf(stderr,"%sMemSocket ERR%s " FMT "\n", COL_RED,    COL_RESET, __VA_ARGS__)
#define TLOG_DBG(FMT,...) IF_DEBUG(fprintf(stderr,"%sMemSocket DBG%s " FMT "\n", COL_CYAN,   COL_RESET, __VA_ARGS__))

#define SLOG_INF(EXPR) do { std::cerr << COL_BLUE   << "MemSocket INF " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_WAR(EXPR) do { std::cerr << COL_YELLOW << "MemSocket WAR " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_ERR(EXPR) do { std::cerr << COL_RED    << "MemSocket ERR " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_DBG(EXPR) IF_DEBUG(do { std::cerr << COL_CYAN   << "MemSocket DBG " << COL_RESET << EXPR << "\n"; } while(0))

#define THROW_ERR(EXPR) do { std::stringstream ss; ss << EXPR; TLOG_ERR("%s", ss.str().c_str()); throw std::runtime_error(ss.str()); } while(0)
