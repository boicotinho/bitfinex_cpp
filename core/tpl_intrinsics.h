#pragma once
#include <stdint.h>
#include <x86intrin.h>

namespace tpl_intrinsics
{
template<class SS, class VV> struct CmpEq { };
#if defined(__AVX2__)
    template<> struct CmpEq< uint8_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi8(aa, bb);} };
    template<> struct CmpEq<  int8_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi8(aa, bb);} };
    template<> struct CmpEq<uint16_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi16(aa, bb);} };
    template<> struct CmpEq< int16_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi16(aa, bb);} };
    template<> struct CmpEq<uint32_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi32(aa, bb);} };
    template<> struct CmpEq< int32_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi32(aa, bb);} };
    template<> struct CmpEq<uint64_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi64(aa, bb);} };
    template<> struct CmpEq< int64_t, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_cmpeq_epi64(aa, bb);} };
#else
    template<class SS> struct CmpEq< SS, __m256i> { static __m256i op(__m256i aa, __m256i bb) {return _mm256_setzero_si256();} };
#endif

template<class VV> struct MoveMask { };
#if defined(__AVX2__)
    template<> struct MoveMask<__m256i> { static uint32_t op(__m256i aa) {return _mm256_movemask_epi8(aa);} };
#else
    template<> struct MoveMask<__m256i> { static uint32_t op(__m256i aa) {return 0;} };
#endif

template<class SS, class VV> struct Broadcast { };
template<> struct Broadcast< uint8_t, __m256i> { static __m256i op( uint8_t aa) {return _mm256_set1_epi8  (aa);} };
template<> struct Broadcast<  int8_t, __m256i> { static __m256i op(  int8_t aa) {return _mm256_set1_epi8  (aa);} };
template<> struct Broadcast<uint16_t, __m256i> { static __m256i op(uint16_t aa) {return _mm256_set1_epi16 (aa);} };
template<> struct Broadcast< int16_t, __m256i> { static __m256i op( int16_t aa) {return _mm256_set1_epi16 (aa);} };
template<> struct Broadcast<uint32_t, __m256i> { static __m256i op(uint32_t aa) {return _mm256_set1_epi32 (aa);} };
template<> struct Broadcast< int32_t, __m256i> { static __m256i op( int32_t aa) {return _mm256_set1_epi32 (aa);} };
template<> struct Broadcast<uint64_t, __m256i> { static __m256i op(uint64_t aa) {return _mm256_set1_epi64x(aa);} };
template<> struct Broadcast< int64_t, __m256i> { static __m256i op( int64_t aa) {return _mm256_set1_epi64x(aa);} };

} // namespace tpl_intrinsics
