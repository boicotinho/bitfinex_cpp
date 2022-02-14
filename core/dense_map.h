#pragma once

// Disabled for now as a safety measure.
//      -- How should we deal with -march=native?
//#if defined(__AVX2__)
//    #define DENSE_MAP_SUPPORTED 1
//#endif

#if !(DENSE_MAP_SUPPORTED)
    #include <unordered_map>
    template<class Traits>
    using DenseMap = std::unordered_map<typename Traits::Key, typename Traits::Value>;
#else
    #include "core/x_assert.h"
    #include "core/t_uint.h"
    #include "core/gcc_utils.h"
    #include "core/hashers.h"
    #include "core/vreg_avx2.h"
    #include <array>
    #include <sstream>
    #include <vector>
    #include <limits>
    #include <type_traits>
    #include <stdint.h>
    #include <stddef.h>
    #include <x86intrin.h>

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
            m_keys.fill(VRegK(encode_key(EMPTY_KEY)));
        }

        // Latency critical function.
        // Checks if key exists
        bool contains(Key kk) const
        {
            const auto ofs_mask = offset_and_mask_find(kk);
            return ofs_mask.second != 0;
        }

        // Latency critical function.
        // Finds key and return index. Will return NOT_FOUND if key doesn't exist.
        idx_t find_ix(Key kk) const
        {
            const auto ofs_mask = offset_and_mask_find(kk);
            if(!ofs_mask.second)
                return NOT_FOUND;
            const auto ofs_within_vector = (__builtin_ffs(ofs_mask.second) - 1) / sizeof(Key);
            return ofs_mask.first + ofs_within_vector;
        }

        // Not currently latency critical.
        std::pair<idx_t, bool> insert(Key kk)
        {
            ASSERT(kk != EMPTY_KEY);
            const idx_t ixf = find_ix(kk);
            if(ixf != NOT_FOUND)
                return {ixf, false};
            const size_t ix0 = 1 == NUM_VREGS ? 0 : hash_index_of_key(kk);
            const size_t ixE = m_keys.size() * VLEN;
            const Key empty = encode_key(EMPTY_KEY);
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
        void for_each(FUNC fn) const
            {
            const Key empty = encode_key(EMPTY_KEY);
            for(idx_t ii = 0; ii < NUM_BUCKETS; ++ii)
            {
                const Key& encoded_key = Keys()[ii];
                if(empty == encoded_key)
                    continue;
                const Key key = decode_key(encoded_key);
                fn(key, ii);
            }
            }

        size_t scan_number_of_keys() const
            {
            size_t sum = 0;
            for_each([&](Key, idx_t){++sum;});
            return sum;
            }

    public: // Helpers

        static idx_t hash_index_of_key(Key kk)
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
        std::pair<size_t, uint32_t> offset_and_mask_find(Key kk) const
        {
            const VRegK vkk = VRegK::MakeBroadcast(encode_key(kk));
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
                constexpr VRegK VEMPTY(encode_key(EMPTY_KEY)); // should be zero
                // Need unaligned load, and need to compare against empty so we can halt before end
                const size_t ix0 = hash_index_of_key(kk);
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
        static constexpr key_t encode_key(key_t kk) {return 1 == NUM_VREGS ? kk : (kk - EMPTY_KEY);}
        static constexpr key_t decode_key(key_t kk) {return 1 == NUM_VREGS ? kk : (kk + EMPTY_KEY);}
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
        bool contains(Key kk) const {return m_index.contains(kk);}

        // Latency-critical member
        const Value* lookup_ptr(Key kk) const
            {
            const idx_t ix = m_index.find_ix(kk);
            if(DenseIdx::NOT_FOUND == ix)
                return nullptr;
            return ValueAt(ix);
            }

        Value* lookup_ptr(Key kk)
            {
            const idx_t ix = m_index.find_ix(kk);
            if(DenseIdx::NOT_FOUND == ix)
                return nullptr;
            return ValueAt(ix);
            }

        // No latency critical use case for now...
        template<class...CtorArgs>
        std::pair<Value*, bool> emplace(Key kk, CtorArgs&&... args)
            {
            auto const ix_b   = m_index.insert(kk);
            const bool is_new = ix_b.second;
            Value* const pval = ValueAt(ix_b.first);
            if(is_new) ValueConstruct(pval, std::forward<CtorArgs>(args)...);
            return {pval, is_new};
            }

        std::pair<Value*, bool> insert(std::pair<Key,Value>&& kvp)
            {
            return emplace(kvp.first, kvp.second);
            }

        Value& operator[] (Key kk) {return *emplace(kk).first;}

        size_t size() const {return m_index.scan_number_of_keys();}

        ~DenseMap()
            {
            m_index.for_each([&](Key kk, idx_t ix)
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

#endif // DENSE_MAP_SUPPORTED
