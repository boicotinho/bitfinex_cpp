#include "test_utils.h"
#include "core/profile_utils.h"
#include "bitfinex/types.h"

#include "core/x_assert.h"
#include "core/t_uint.h"
#include "core/gcc_utils.h"
#include "core/hashers.h"
#include "core/vreg_avx2.h"
#include <array>
#include <vector>
#include <limits>
#include <type_traits>
#include <stdint.h>
#include <stddef.h>
#include <x86intrin.h>

// TODO:    1) Test UT in AVX2 laptop 
//          2) Put DenseMap in class
//          3) If AVX2 not enabled (march=native), then use normal unordered map, maybe flat version of it
//          4) Profile test
//
//          5) Finish up Listening class

#pragma GCC diagnostic ignored "-Wignored-attributes"

#define THROW_CERR(EXPR) do { std::stringstream ss; ss << EXPR; throw std::runtime_error(ss.str()); } while(0)

struct ExampleTraits__DenseIndex_and_Map
{
    using Key    = uint8_t;
    using Hasher = IdentityHasher<Key>;
    enum { EMPTY_KEY       = 255 };
    enum { MIN_NUM_BUCKETS = 32  };
    using Value  = uint32_t;
};

// key -> index map, does not provide value storage
// Static number of buckets
template< class T_Traits >
class DenseIndex
{
public:
    using Key     = typename T_Traits::Key;
    using Hasher  = typename T_Traits::Hasher;
    using VRegK   = cpu_avx2::VReg<Key>;
    enum : Key    { EMPTY_KEY   = T_Traits::EMPTY_KEY};
    enum : size_t { VLEN        = VRegK::LEN };
    enum : size_t { NUM_BUCKETS = (size_t)T_Traits::MIN_NUM_BUCKETS <= (size_t)VLEN ? VLEN : ALIGN_UP(T_Traits::MIN_NUM_BUCKETS + VLEN, VLEN) };
    using idx_t   = uintv<NUM_BUCKETS>;
    enum : idx_t  { NOT_FOUND   = NUM_BUCKETS};
    enum : size_t { NUM_VREGS   = NUM_BUCKETS / VLEN };
    enum : size_t { NUM_SPILLV  = NUM_VREGS > 1 ? 1 : 0};

    DenseIndex()
    {
        ASSERT_ALIGNED(this,sizeof(VRegK));
        m_keys.fill(VRegK(EncodeKey(EMPTY_KEY)));
    }

    // Latency critical function.
    // Checks if key exists
    bool Contains(Key kk) const
    {
        const auto ofs_mask = OffsetAndMaskFind(kk);
        return ofs_mask.second != 0;
    }

    // Latency critical function.
    // Finds key and return index. Will return NOT_FOUND if key doesn't exist.
    idx_t Find(Key kk) const
    {
        const auto ofs_mask = OffsetAndMaskFind(kk);
        if(!ofs_mask.second)
            return NOT_FOUND;
        const auto ofs_within_vector = (__builtin_ffs(ofs_mask.second) - 1) / sizeof(Key);
        return ofs_mask.first + ofs_within_vector;
    }

    // Not currently latency critical.
    std::pair<idx_t, bool> Insert(Key kk)
    {
        ASSERT(kk != EMPTY_KEY);
        const idx_t ixf = Find(kk);
        if(ixf != NOT_FOUND)
            return {ixf, false};
        const size_t ix0 = 1 == NUM_VREGS ? 0 : HashIndexOfKey(kk);
        const size_t ixE = m_keys.size() * VLEN;
        const Key empty = EncodeKey(EMPTY_KEY);
        const auto keys = Keys();
        for(size_t ii = ix0; ii < ixE; ++ii)
        {
            if(keys[ii] == empty)
            {
                keys[ii] = kk;
                return {ii, true};
            }
        }
        THROW_CERR("DenseIndex capacity (" << NUM_BUCKETS
            << ") overflow when inserting key=" << kk);
    }

    template<class FUNC>
    void ForEach(FUNC fn) const
        {
        const Key empty = EncodeKey(EMPTY_KEY);
        for(idx_t ii = 0; ii < NUM_BUCKETS; ++ii)
        {
            const Key& encoded_key = Keys()[ii];
            if(empty == encoded_key)
                continue;
            const Key key = DecodeKey(encoded_key);
            fn(key, ii);
        }
        }

    size_t ScanNumKeys() const
        {
        size_t sum = 0;
        ForEach([&](Key, idx_t){++sum;});
        return sum;
        }

public: // Helpers

    static idx_t HashIndexOfKey(Key kk)
    {
        // no point in hashing index if we only have 1 vector register,
        // because search with a vector happens in parallel and costs the same.
        // Also having
        if(1 == NUM_VREGS)
            return 0;
        Hasher hasher;
        const size_t hh = hasher(kk);
        const idx_t  ix = hh % NUM_BUCKETS;
        return ix;
    }

    // return (scalar_ix, eq_bitmask)
    // if not found, eq_bitmask will be 0
    std::pair<size_t, uint32_t> OffsetAndMaskFind(Key kk) const
    {
        const VRegK vkk = VRegK::MakeBroadcast(EncodeKey(kk));
        if(1 == NUM_VREGS)
        {
            // If just 1 vector, then use aligned load and don't bother
            // encoding/hashing key or comparing against empty slots.
            const VRegK     veq = m_keys[0].CmpEq(vkk);
            const uint32_t  msk = veq.MoveMask();
            return {0, msk};
        }
        else
        {
            constexpr VRegK VEMPTY(EncodeKey(EMPTY_KEY)); // should be zero
            // Need unaligned load, and need to compare against empty so we can halt before end
            const size_t ix0 = HashIndexOfKey(kk);
            const Key* const bgn = Keys() + ix0;
            const Key* const end = &(m_keys[NUM_VREGS-NUM_SPILLV].scalars[0]);
            for(const Key* pp = bgn; pp < end; pp += VLEN)
            {
                using MoveMask = tpl_intrinsics::MoveMask<typename VRegK::vector_t>;
                // Check key vector against given key (32 bytes at a time)
                const VRegK     src = VRegK::LoadUnaligned(pp);
                const VRegK     veq = src.CmpEq(vkk);
                const uint32_t  msk = MoveMask::op(veq); // AVX2-only, for now
                if(msk)
                    return {pp - Keys(), msk}; // found
                // If there's empty slot then return not found
                const VRegK     v00 = src.CmpEq(VEMPTY);
                const uint32_t  m00 = MoveMask::op(v00);  // AVX2-only, for now
                if(m00)
                    break;
            }
            return {0, 0};
        }
    }

private:
    static constexpr key_t EncodeKey(key_t kk) {return 1 == NUM_VREGS ? kk : (kk - EMPTY_KEY);}
    static constexpr key_t DecodeKey(key_t kk) {return 1 == NUM_VREGS ? kk : (kk + EMPTY_KEY);}
    Key* Keys() const {return (Key*)&m_keys;}

private:
    std::array<VRegK, NUM_VREGS> m_keys;
};


template< class T_Traits >
class DenseMap
{
public:
    using Value     = typename T_Traits::Value;
    using Key       = typename T_Traits::Key;
    enum : size_t   { NUM_BUCKETS = DenseIndex<T_Traits>::NUM_BUCKETS };

public:
    // Latency-critical member
    bool Contains(Key kk) const {return m_index.Contains(kk);}

    // Latency-critical member
    const Value* Lookup(Key kk) const
        {
        const idx_t ix = m_index.Find(kk);
        if(DenseIdx::NOT_FOUND == ix)
            return nullptr;
        return ValueAt(ix);
        }

    // No latency critical use case for now...
    template<class...CtorArgs>
    std::pair<Value*, bool> Emplace(Key kk, CtorArgs&&... args)
        {
        auto const ix_b   = m_index.Insert(kk);
        const bool is_new = ix_b.second;
        Value* const pval = ValueAt(ix_b.first);
        if(is_new) ValueConstruct(pval, std::forward<CtorArgs>(args)...);
        return {pval, is_new};
        }

    Value& operator[] (Key kk) {return *Emplace(kk).first;}

    size_t Size() const {return m_index.ScanNumKeys();}

    ~DenseMap()
        {
        m_index.ForEach([&](Key kk, idx_t ix)
        {
            Value* const pval = ValueAt(ix);
            ValueDestroy(pval);
        });
        }

private:
    template<class...CtorArgs>
    void ValueConstruct(Value* pp, CtorArgs&&... args) { new(pp) Value(std::forward<CtorArgs>(args)...);}
    void ValueDestroy(Value* pp) { pp->Value::~Value();}
    Value* ValueAt(size_t ix) const
        {
        ASSERT_LT(ix, m_values.size());
        return (Value*) &m_values[ix];
        }

private:
    using ValStore  = typename std::aligned_storage< sizeof(Value), alignof(Value) >::type;
    using DenseIdx  = DenseIndex<T_Traits>;
    using idx_t     = typename DenseIdx::idx_t;

private:
    DenseIdx                            m_index;
    std::array<ValStore, NUM_BUCKETS>   m_values;
};

BOOST_AUTO_TEST_SUITE(bitfinex)

BOOST_AUTO_TEST_CASE(dense_map)
{
    struct TagToBookMapTraits
    {
        using Key    = uint32_t; // tag
        using Hasher = IdentityHasher<Key>;
        enum { EMPTY_KEY       = 0 };
        enum { MIN_NUM_BUCKETS = 32  };
        using Value  = uint32_t;
    };

    DenseMap<TagToBookMapTraits> map;

    map.Emplace(11, 111);
    map.Emplace(22, 222);
    map.Emplace(33, 222);

    auto pval = map.Lookup(22);
    BOOST_REQUIRE(pval);
    BOOST_CHECK_EQUAL(*pval, 222);
    BOOST_CHECK_EQUAL(3, 7);
}

BOOST_AUTO_TEST_SUITE_END()
