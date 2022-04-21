#pragma once
#include "gcc_utils.h"
#include "swallow_exceptions.h"

// Declare code block to run when exiting current scope
// (good for tidying up from early returns)
// Note: Avoid throwing exceptions from (FN) body, because it
// will be called from a destructor and exceptions will be swollen.
#define SCOPE_EXIT(FN) \
    auto UNIQUE_VAR(__xx_scope_exit_) = detail::make_scope_exit( [&]() FN )

namespace detail {

template <typename Func>
struct ScopeExit
{
    ScopeExit(const ScopeExit&) = default;
    ScopeExit(ScopeExit&&) = default;
    ScopeExit(Func ff) : func(ff) {}
    ~ScopeExit() { SWALLOW_EXCEPTIONS(func();); }
    Func func;
};

template <typename Func>
ScopeExit<Func> make_scope_exit(Func ff)
{
    return ScopeExit<Func>(std::forward<Func>(ff));
}

} // namespace detail
