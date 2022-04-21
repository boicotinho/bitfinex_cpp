// This is a mix between std::vector and std::array
// It saves a pointer deref, dynamic size up to a limit,
// and implements for(auto&: array_sz) semantics

#pragma once

#include "t_uint.h"
#include "x_assert.h"
#include <array>

template<class T, size_t MAX_N>
class array_sz
{
public:
    using size_type = uintv<MAX_N>;

    enum {INVOKE_DESTRUCTORS = false};

    static size_t capacity() {return MAX_N;}
    size_t size() const {return m_size;}
    void resize(size_t sz) {ASSERT_LE(sz,MAX_N); m_size = sz;} // doesn't invoke destructor
    void clear() { resize(0);}

    void fill(const T& val)   { std::fill(m_arr.begin(), m_arr.end(), val); }

    void dpush_back(const T& v) {
        ASSERT_LT(m_size, MAX_N);
        m_arr[m_size++]=v;
    }

    T& demplace_back() // doesn't actually invoke constructor for now
    {
        ASSERT_LT(m_size, MAX_N);
        return m_arr[m_size++];
    }

    T& operator[](size_t ix)             {ASSERT_LT(ix,m_size); return m_arr[ix];}
    const T& operator[](size_t ix) const {ASSERT_LT(ix,m_size); return m_arr[ix];}

    using array_t        = std::array<T, MAX_N>;
    using iterator       = typename array_t::iterator;
    using const_iterator = typename array_t::const_iterator;

    iterator begin()              {return m_arr.begin();}
    iterator end()                {return begin()+m_size;}
    const_iterator begin() const  {return m_arr.cbegin();}
    const_iterator end()   const  {return m_arr.cbegin()+m_size;}
    const_iterator cbegin() const {return m_arr.cbegin();}
    const_iterator cend()   const {return m_arr.cbegin()+m_size;}

private:
    size_type   m_size {0};
    array_t     m_arr {{}};
};
