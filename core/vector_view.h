#pragma once
#include "x_assert.h"
#include <vector>

template<class TVal>
class VectorView
{
public:
    using Base = std::vector<TVal>;

    VectorView() : m_view {nullptr, nullptr, nullptr} {}

    VectorView(Base const& base, size_t ii_bgn, size_t ii_end)
        : m_view{ base.data() + ii_bgn
                , base.data() + ii_end
                , base.data() + ii_end }
        {
        ASSERT(ii_bng <  base.size());
        ASSERT(ii_end <= base.size());
        ASSERT(ii_bng <= ii_end);
        }

    void set_view_length(size_t new_len)
        {
        ASSERT(m_view.bgn);
        ASSERT(m_view.bgn + new_len <= m_view.end);
        m_view.end = m_view.bgn + new_len;
        }

    Base const& get() const {Caster cc; cc.view = this; return *cc.base; }

private:
    struct View { TVal* bgn; TVal* end; TVal* rsv;};
    union Caster { Base* base; View* view; };
    View m_view;
};
