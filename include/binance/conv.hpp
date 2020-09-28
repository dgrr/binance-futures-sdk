#ifndef BINANCE_CONV_HPP
#define BINANCE_CONV_HPP

#include <immintrin.h>

#include <binance/common.hpp>
#include <binance/error.hpp>
#include <string>
#include <string_view>

namespace binance
{
namespace conv
{
really_inline int64_t parse_int(const char* s, std::size_t remaining)
{
  int64_t val = 0;
  while (remaining >= 8)
  {
    // Broadcast 8-bit integer a to all elements of dst.
    __m128i ascii0 = _mm_set1_epi8('0');
    // Set packed 8-bit integers in dst with the supplied values in reverse
    // order.
    __m128i mul_1_10 =
        _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
    // Set packed 16-bit integers in dst with the supplied values in reverse
    // order.
    __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
    // Set packed 16-bit integers in dst with the supplied values in reverse
    // order.
    __m128i mul_1_10000 =
        _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
    __m128i in = _mm_sub_epi8(_mm_loadu_si128((__m128i*) s), ascii0);
    // Vertically multiply each unsigned 8-bit integer from a with the
    // corresponding signed 8-bit integer from b
    __m128i t1 = _mm_maddubs_epi16(in, mul_1_10);
    // Multiply packed signed 16-bit integers in a and b, producing intermediate
    // signed 32-bit integers
    __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
    // Convert packed signed 32-bit integers from a and b to packed 16-bit
    // integers using unsigned saturation
    __m128i t3 = _mm_packus_epi32(t2, t2);
    // Multiply packed signed 16-bit integers in a and b, producing intermediate
    // signed 32-bit integers.
    __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
    // Copy the lower 32-bit integer in a to dst.
    val = val * 100000000 + _mm_cvtsi128_si32(t4);
    remaining -= 8;
    s += 8;
  }
  while (remaining-- > 0)
    val = val * 10 + (*s++ - '0');

  return val;
}

really_inline int64_t parse_int(const char* s)
{
  return std::atol(&s[0]);
}

really_inline int64_t parse_int(const std::string& s)
{
  return parse_int((const char*) &s[0], s.size());
}

// TODO: use SIMD conversion for floats?
really_inline double parse_float(const std::string& s)
{
  return std::strtod(&s[0], nullptr);
}

static const unsigned char __hex_chars[16]   = {'0', '1', '2', '3', '4', '5',
                                              '6', '7', '8', '9', 'a', 'b',
                                              'c', 'd', 'e', 'f'};
static const unsigned char __blend_table[32] = {
    0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128,
    0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128};
static const unsigned char __dup_index[16] = {0, 0, 1, 1, 2, 2, 3, 3,
                                              4, 4, 5, 5, 6, 6, 7, 7};

class hex
{
  std::string r_;
  __m128i table_;
  __m128i mask_;
  __m128i dup_index_;

public:
  hex()
  {
    table_     = _mm_loadu_si128((__m128i const*) &__hex_chars[0]);
    mask_      = _mm_loadu_si128((__m128i const*) &__blend_table[0]);
    dup_index_ = _mm_loadu_si128((__m128i const*) &__dup_index[0]);
  }

  hex& clear()
  {
    r_.clear();
    return *this;
  }

  hex& encode(const unsigned char* data, size_t size)
  {
    __m128i _s     = _mm_set1_epi8(15);
    size_t r_index = r_.size();
    r_.resize(r_.size() + size * 2);

    size_t i = 0;
    for (; size >= 8; i += 8, r_index += 16, size -= 8)
    {
      __m128i _r, _dh, _dl;                                    // result
      __m128i _data = _mm_loadu_si64((void const*) &data[i]);  // read 8 bytes

      _dh = _mm_srli_epi16(_data, 4);       // shift right 4 bits
      _dh = _mm_and_si128(_dh, _s);         // remove higher bits
      _dh = _mm_shuffle_epi8(table_, _dh);  // search in table

      _dl = _mm_and_si128(_data, _s);       // remove higher bits
      _dl = _mm_shuffle_epi8(table_, _dl);  // search in table

      _dh = _mm_shuffle_epi8(_dh, dup_index_);  // duplicate first
      _dl = _mm_shuffle_epi8(_dl, dup_index_);  // duplicate second

      _r = _mm_blendv_epi8(_dh, _dl, mask_);  // blend

      _mm_storeu_si128((__m128i*) &r_[r_index], _r);  // store in r_
    }
    while (size > 0)
    {
      r_[r_index++] = __hex_chars[data[i] >> 4];
      r_[r_index++] = __hex_chars[data[i++] & 15];
      size--;
    }

    return *this;
  }
  const std::string& final() const
  {
    return r_;
  }
};
}  // namespace conv
}  // namespace binance

#endif
