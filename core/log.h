#pragma once
#include "gcc_utils.h"
#include "color.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#define TLOG_INF(FMT,...) fprintf(stderr,"%scpp-core INF%s " FMT "\n", COL_BLUE,   COL_RESET, __VA_ARGS__)
#define TLOG_WAR(FMT,...) fprintf(stderr,"%scpp-core WAR%s " FMT "\n", COL_YELLOW, COL_RESET, __VA_ARGS__)
#define TLOG_ERR(FMT,...) fprintf(stderr,"%scpp-core ERR%s " FMT "\n", COL_RED,    COL_RESET, __VA_ARGS__)
#define TLOG_DBG(FMT,...) IF_DEBUG(fprintf(stderr,"%cpp-core DBG%s " FMT "\n", COL_CYAN,   COL_RESET, __VA_ARGS__))

#define SLOG_INF(EXPR)  do { std::cerr << COL_BLUE   << "cpp-core INF " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_WAR(EXPR)  do { std::cerr << COL_YELLOW << "cpp-core WAR " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_ERR(EXPR)  do { std::cerr << COL_RED    << "cpp-core ERR " << COL_RESET << EXPR << "\n"; } while(0)
#define SLOG_DBG(EXPR)  IF_DEBUG(do { std::cerr << COL_CYAN   << "cpp-core DBG " << COL_RESET << EXPR << "\n"; } while(0) )
