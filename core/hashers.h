#pragma once
#include <string>
#include <stdint.h>
#include <x86intrin.h>

// The exact algorithm is actually CRC-32C (Castagnoli) rather than CRC-32.
// Used in iSCSI, SCTP, G.hn payload, SSE4.2, Btrfs, ext4, Ceph
// The version by cksum linux tool is CRC-32
// https://en.wikipedia.org/wiki/Cyclic_redundancy_check
// https://stackoverflow.com/questions/29174349/mm-crc32-u8-gives-different-result-than-reference-code
inline uint32_t CalcCrc32C(uint32_t crc, uint8_t  val) {return _mm_crc32_u8(crc, val);}
inline uint32_t CalcCrc32C(uint32_t crc, uint16_t val) {return _mm_crc32_u16(crc, val);}
inline uint32_t CalcCrc32C(uint32_t crc, uint32_t val) {return _mm_crc32_u32(crc, val);}
inline uint64_t CalcCrc32C(uint64_t crc, uint64_t val) {return _mm_crc32_u64(crc, val);}

inline uint32_t CalcCrc32C(uint32_t crc, int8_t  val) {return _mm_crc32_u8(crc, val);}
inline uint32_t CalcCrc32C(uint32_t crc, int16_t val) {return _mm_crc32_u16(crc, val);}
inline uint32_t CalcCrc32C(uint32_t crc, int32_t val) {return _mm_crc32_u32(crc, val);}
inline uint64_t CalcCrc32C(uint64_t crc, int64_t val) {return _mm_crc32_u64(crc, val);}

template<class TT> struct IdentityHasher;
template<> struct IdentityHasher< uint8_t> { size_t operator()( uint8_t vv) const {return vv;}};
template<> struct IdentityHasher<  int8_t> { size_t operator()(  int8_t vv) const {return vv;}};
template<> struct IdentityHasher<uint16_t> { size_t operator()(uint16_t vv) const {return vv;}};
template<> struct IdentityHasher< int16_t> { size_t operator()( int16_t vv) const {return vv;}};
template<> struct IdentityHasher<uint32_t> { size_t operator()(uint32_t vv) const {return vv;}};
template<> struct IdentityHasher< int32_t> { size_t operator()( int32_t vv) const {return vv;}};
template<> struct IdentityHasher<uint64_t> { size_t operator()(uint64_t vv) const {return vv;}};
template<> struct IdentityHasher< int64_t> { size_t operator()( int64_t vv) const {return vv;}};

template<class TT> struct Crc32Hasher;
template<> struct Crc32Hasher< uint8_t> { size_t operator()( uint8_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher<  int8_t> { size_t operator()(  int8_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher<uint16_t> { size_t operator()(uint16_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher< int16_t> { size_t operator()( int16_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher<uint32_t> { size_t operator()(uint32_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher< int32_t> { size_t operator()( int32_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher<uint64_t> { size_t operator()(uint64_t vv) const {return CalcCrc32C(0, vv);}};
template<> struct Crc32Hasher< int64_t> { size_t operator()( int64_t vv) const {return CalcCrc32C(0, vv);}};
