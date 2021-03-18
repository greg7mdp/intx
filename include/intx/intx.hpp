// intx: extended precision integer library.
// Copyright 2019-2020 Pawel Bylica.
// Licensed under the Apache License, Version 2.0.

#pragma once

#include <intx/int128.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace intx
{
template <unsigned N>
struct uint
{
    static_assert((N & (N - 1)) == 0, "Number of bits must be power of 2");
    static_assert(N >= 256, "Number of bits must be at lest 256");

    using word_type = uint64_t;

    /// The 2x smaller type.
    using half_type = uint<N / 2>;

    static constexpr auto num_bits = N;
    static constexpr auto num_words = N / 8 / sizeof(word_type);

    half_type lo = 0;
    half_type hi = 0;

    constexpr uint() noexcept = default;

    constexpr uint(half_type high, half_type low) noexcept : lo(low), hi(high) {}

    /// Implicit converting constructor for the half type.
    constexpr uint(half_type x) noexcept : lo(x) {}  // NOLINT

    /// Implicit converting constructor for types convertible to the half type.
    template <typename T,
        typename = typename std::enable_if<std::is_convertible<T, half_type>::value>::type>
    constexpr uint(T x) noexcept : lo(x)  // NOLINT
    {}

    constexpr uint64_t& operator[](size_t i) noexcept
    {
        constexpr auto half_words = num_words / 2;
        return (i < half_words) ? this->lo[i] : this->hi[i - half_words];
    }

    constexpr const uint64_t& operator[](size_t i) const noexcept
    {
        constexpr auto half_words = num_words / 2;
        return (i < half_words) ? this->lo[i] : this->hi[i - half_words];
    }

    constexpr explicit operator bool() const noexcept
    {
        return static_cast<bool>(lo) | static_cast<bool>(hi);
    }

    /// Explicit converting operator for all builtin integral types.
    template <typename Int, typename = typename std::enable_if<std::is_integral<Int>::value>::type>
    explicit operator Int() const noexcept
    {
        return static_cast<Int>(lo);
    }
};

using uint256 = uint<256>;
using uint512 = uint<512>;

inline constexpr uint8_t lo(uint16_t x)
{
    return static_cast<uint8_t>(x);
}

inline constexpr uint16_t lo(uint32_t x)
{
    return static_cast<uint16_t>(x);
}

inline constexpr uint32_t lo(uint64_t x)
{
    return static_cast<uint32_t>(x);
}

inline constexpr uint8_t hi(uint16_t x)
{
    return static_cast<uint8_t>(x >> 8);
}

inline constexpr uint16_t hi(uint32_t x)
{
    return static_cast<uint16_t>(x >> 16);
}

inline constexpr uint32_t hi(uint64_t x)
{
    return static_cast<uint32_t>(x >> 32);
}

template <unsigned N>
inline constexpr auto lo(const uint<N>& x) noexcept
{
    return x.lo;
}

template <unsigned N>
inline constexpr auto hi(const uint<N>& x) noexcept
{
    return x.hi;
}

template <typename T>
inline constexpr unsigned num_bits(const T&) noexcept
{
    return sizeof(T) * 8;
}

template <unsigned N>
inline constexpr bool operator==(const uint<N>& a, const uint<N>& b) noexcept
{
    return (a.lo == b.lo) & (a.hi == b.hi);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator==(const uint<N>& x, const T& y) noexcept
{
    return x == uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator==(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(y) == x;
}


template <unsigned N>
inline constexpr bool operator!=(const uint<N>& a, const uint<N>& b) noexcept
{
    return !(a == b);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator!=(const uint<N>& x, const T& y) noexcept
{
    return x != uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator!=(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) != y;
}


template <unsigned N>
inline constexpr bool operator<(const uint<N>& x, const uint<N>& y) noexcept
{
    return sub_with_carry(x, y).carry;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator<(const uint<N>& x, const T& y) noexcept
{
    return x < uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator<(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) < y;
}


template <unsigned N>
inline constexpr bool operator>(const uint<N>& x, const uint<N>& y) noexcept
{
    return sub_with_carry(y, x).carry;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator>(const uint<N>& x, const T& y) noexcept
{
    return x > uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator>(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) > y;
}


template <unsigned N>
inline constexpr bool operator>=(const uint<N>& x, const uint<N>& y) noexcept
{
    return !sub_with_carry(x, y).carry;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator>=(const uint<N>& x, const T& y) noexcept
{
    return x >= uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator>=(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) >= y;
}


template <unsigned N>
inline constexpr bool operator<=(const uint<N>& x, const uint<N>& y) noexcept
{
    return !sub_with_carry(y, x).carry;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator<=(const uint<N>& x, const T& y) noexcept
{
    return x <= uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr bool operator<=(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) <= y;
}

template <unsigned N>
inline constexpr uint<N> operator|(const uint<N>& x, const uint<N>& y) noexcept
{
    uint<N> z;
    for (size_t i = 0; i < uint<N>::num_words; ++i)
        z[i] = x[i] | y[i];
    return z;
}

template <unsigned N>
inline constexpr uint<N> operator&(const uint<N>& x, const uint<N>& y) noexcept
{
    uint<N> z;
    for (size_t i = 0; i < uint<N>::num_words; ++i)
        z[i] = x[i] & y[i];
    return z;
}

template <unsigned N>
inline constexpr uint<N> operator^(const uint<N>& x, const uint<N>& y) noexcept
{
    uint<N> z;
    for (size_t i = 0; i < uint<N>::num_words; ++i)
        z[i] = x[i] ^ y[i];
    return z;
}

template <unsigned N>
inline constexpr uint<N> operator~(const uint<N>& x) noexcept
{
    uint<N> z;
    for (size_t i = 0; i < uint<N>::num_words; ++i)
        z[i] = ~x[i];
    return z;
}

template <unsigned N>
inline constexpr uint<N> operator<<(const uint<N>& x, uint64_t shift) noexcept
{
    constexpr auto num_bits = N;
    constexpr auto half_bits = num_bits / 2;

    if (shift < half_bits)
    {
        const auto lo = x.lo << shift;

        // Find the part moved from lo to hi.
        // The shift right here can be invalid:
        // for shift == 0 => lshift == half_bits.
        // Split it into 2 valid shifts by (rshift - 1) and 1.
        const auto rshift = half_bits - shift;
        const auto lo_overflow = (x.lo >> (rshift - 1)) >> 1;
        const auto hi = (x.hi << shift) | lo_overflow;
        return {hi, lo};
    }

    // This check is only needed if we want "defined" behavior for shifts
    // larger than size of the Int.
    if (shift < num_bits)
        return {x.lo << (shift - half_bits), 0};

    return 0;
}


template <unsigned N>
inline constexpr uint<N> operator>>(const uint<N>& x, uint64_t shift) noexcept
{
    constexpr auto num_bits = N;
    constexpr auto half_bits = num_bits / 2;

    if (shift < half_bits)
    {
        const auto hi = x.hi >> shift;

        // Find the part moved from hi to lo.
        // To avoid invalid shift left,
        // split them into 2 valid shifts by (lshift - 1) and 1.
        const auto lshift = half_bits - shift;
        const auto hi_overflow = (x.hi << (lshift - 1)) << 1;
        const auto lo_part = x.lo >> shift;
        const auto lo = lo_part | hi_overflow;
        return {hi, lo};
    }

    if (shift < num_bits)
        return {0, x.hi >> (shift - half_bits)};

    return 0;
}


template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator<<(const uint<N>& x, const T& shift) noexcept
{
    if (shift < T{sizeof(x) * 8})
        return x << static_cast<uint64_t>(shift);
    return 0;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator>>(const uint<N>& x, const T& shift) noexcept
{
    if (shift < T{sizeof(x) * 8})
        return x >> static_cast<uint64_t>(shift);
    return 0;
}

template <unsigned N>
inline constexpr uint<N>& operator>>=(uint<N>& x, uint64_t shift) noexcept
{
    return x = x >> shift;
}


inline constexpr uint64_t* as_words(uint128& x) noexcept
{
    return &x[0];
}

inline constexpr const uint64_t* as_words(const uint128& x) noexcept
{
    return &x[0];
}

template <unsigned N>
inline constexpr uint64_t* as_words(uint<N>& x) noexcept
{
    return as_words(x.lo);
}

template <unsigned N>
inline constexpr const uint64_t* as_words(const uint<N>& x) noexcept
{
    return as_words(x.lo);
}

template <unsigned N>
inline uint8_t* as_bytes(uint<N>& x) noexcept
{
    return reinterpret_cast<uint8_t*>(as_words(x));
}

template <unsigned N>
inline const uint8_t* as_bytes(const uint<N>& x) noexcept
{
    return reinterpret_cast<const uint8_t*>(as_words(x));
}

/// Implementation of shift left as a loop.
/// This one is slower than the one using "split" strategy.
template <unsigned N>
inline uint<N> shl_loop(const uint<N>& x, uint64_t shift)
{
    auto r = uint<N>{};
    constexpr unsigned word_bits = sizeof(uint64_t) * 8;
    constexpr size_t num_words = sizeof(uint<N>) / sizeof(uint64_t);
    auto rw = as_words(r);
    auto words = as_words(x);
    const auto s = shift % word_bits;
    const auto skip = shift / word_bits;

    uint64_t carry = 0;
    for (size_t i = 0; i < (num_words - skip); ++i)
    {
        auto w = words[i];
        auto v = (w << s) | carry;
        carry = (w >> (word_bits - s - 1)) >> 1;
        rw[i + skip] = v;
    }
    return r;
}

template <unsigned N>
inline constexpr uint<N> add_loop(const uint<N>& x, const uint<N>& y) noexcept
{
    uint<N> s;
    bool k = false;
    for (size_t i = 0; i < uint<N>::num_words; ++i)
    {
        s[i] = x[i] + y[i];
        auto k1 = s[i] < x[i];
        s[i] += k;
        k = (s[i] < k) || k1;
    }
    return s;
}

template <unsigned N>
inline constexpr uint<N> operator+(const uint<N>& x, const uint<N>& y) noexcept
{
    return add_with_carry(x, y).value;
}

template <unsigned N>
inline constexpr uint<N> operator-(const uint<N>& x) noexcept
{
    return ~x + uint<N>{1};
}

template <unsigned N>
inline constexpr uint<N> operator-(const uint<N>& x, const uint<N>& y) noexcept
{
    return sub_with_carry(x, y).value;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator+=(uint<N>& x, const T& y) noexcept
{
    return x = x + y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator-=(uint<N>& x, const T& y) noexcept
{
    return x = x - y;
}


template <unsigned N>
inline constexpr uint<2 * N> umul(const uint<N>& x, const uint<N>& y) noexcept
{
    const auto t0 = umul(lo(x), lo(y));
    const auto t1 = umul(hi(x), lo(y));
    const auto t2 = umul(lo(x), hi(y));
    const auto t3 = umul(hi(x), hi(y));

    const auto u1 = t1 + hi(t0);
    const auto u2 = t2 + lo(u1);

    const auto l = (u2 << (num_bits(x) / 2)) | lo(t0);
    const auto h = t3 + hi(u2) + hi(u1);

    return {h, l};
}

template <unsigned N>
inline constexpr uint<N> sqr(const uint<N>& x) noexcept
{
    // Based on recursive multiplication implementation.

    const auto t = umul(lo(x), lo(x));
    const auto h = ((lo(x) * hi(x)) << 1) + hi(t);

    return {h, lo(t)};
}


template <unsigned N>
inline uint<2 * N> umul_loop(const uint<N>& x, const uint<N>& y) noexcept
{
    constexpr auto num_words = sizeof(uint<N>) / sizeof(uint64_t);

    uint<2 * N> p;
    auto pw = as_words(p);
    const auto xw = as_words(x);
    const auto yw = as_words(y);

    for (size_t j = 0; j < num_words; ++j)
    {
        uint64_t k = 0;
        for (size_t i = 0; i < num_words; ++i)
        {
            const auto t = umul(xw[i], yw[j]) + pw[i + j] + k;
            pw[i + j] = lo(t);
            k = hi(t);
        }
        pw[j + num_words] = k;
    }
    return p;
}

/// Multiplication implementation using word access
/// and discarding the high part of the result product.
template <unsigned N>
inline constexpr uint<N> operator*(const uint<N>& x, const uint<N>& y) noexcept
{
    constexpr auto num_words = uint<N>::num_words;

    uint<N> p;
    for (size_t j = 0; j < num_words; j++)
    {
        uint64_t k = 0;
        for (size_t i = 0; i < (num_words - j - 1); i++)
        {
            const auto t = umul(x[i], y[j]) + p[i + j] + k;
            p[i + j] = lo(t);
            k = hi(t);
        }
        p[num_words - 1] += x[num_words - j - 1] * y[j] + k;
    }
    return p;
}


template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator*=(uint<N>& x, const T& y) noexcept
{
    return x = x * y;
}

template <unsigned N>
inline constexpr uint<N> exp(uint<N> base, uint<N> exponent) noexcept
{
    auto result = uint<N>{1};
    if (base == 2)
        return result << exponent;

    while (exponent != 0)
    {
        if ((exponent & 1) != 0)
            result *= base;
        base = sqr(base);
        exponent >>= 1;
    }
    return result;
}

template <unsigned N>
inline constexpr unsigned clz(const uint<N>& x) noexcept
{
    const auto half_bits = num_bits(x) / 2;

    // TODO: Try:
    // bool take_hi = h != 0;
    // bool take_lo = !take_hi;
    // unsigned clz_hi = take_hi * clz(h);
    // unsigned clz_lo = take_lo * (clz(l) | half_bits);
    // return clz_hi | clz_lo;

    // In this order `h == 0` we get less instructions than in case of `h != 0`.
    return x.hi == 0 ? clz(x.lo) + half_bits : clz(x.hi);
}

template <typename Word, typename Int>
std::array<Word, sizeof(Int) / sizeof(Word)> to_words(Int x) noexcept
{
    std::array<Word, sizeof(Int) / sizeof(Word)> words;
    std::memcpy(&words, &x, sizeof(x));
    return words;
}

template <typename Word>
unsigned count_significant_words_loop(uint256 x) noexcept
{
    auto words = to_words<Word>(x);
    for (size_t i = words.size(); i > 0; --i)
    {
        if (words[i - 1] != 0)
            return static_cast<unsigned>(i);
    }
    return 0;
}

template <typename Word, typename Int>
inline typename std::enable_if<sizeof(Word) == sizeof(Int), unsigned>::type count_significant_words(
    const Int& x) noexcept
{
    return x != 0 ? 1 : 0;
}

template <typename Word, typename Int>
inline typename std::enable_if<sizeof(Word) < sizeof(Int), unsigned>::type count_significant_words(
    const Int& x) noexcept
{
    constexpr auto num_words = static_cast<unsigned>(sizeof(x) / sizeof(Word));
    auto h = count_significant_words<Word>(hi(x));
    auto l = count_significant_words<Word>(lo(x));
    return h != 0 ? h + (num_words / 2) : l;
}


namespace internal
{
/// Counts the number of zero leading bits in nonzero argument x.
inline constexpr unsigned clz_nonzero(uint64_t x) noexcept
{
    INTX_REQUIRE(x != 0);
#ifdef _MSC_VER
    return clz_generic(x);
#else
    return unsigned(__builtin_clzll(x));
#endif
}

template <unsigned N>
struct normalized_div_args
{
    uint<N> divisor;
    uint<N> numerator;
    typename uint<N>::word_type numerator_ex;
    int num_divisor_words;
    int num_numerator_words;
    unsigned shift;
};

template <typename IntT>
[[gnu::always_inline]] inline normalized_div_args<IntT::num_bits> normalize(
    const IntT& numerator, const IntT& denominator) noexcept
{
    // FIXME: Make the implementation type independent
    static constexpr auto num_words = IntT::num_words;

    auto* u = as_words(numerator);
    auto* v = as_words(denominator);

    normalized_div_args<IntT::num_bits> na;
    auto* un = as_words(na.numerator);
    auto* vn = as_words(na.divisor);

    auto& m = na.num_numerator_words;
    for (m = num_words; m > 0 && u[m - 1] == 0; --m)
        ;

    auto& n = na.num_divisor_words;
    for (n = num_words; n > 0 && v[n - 1] == 0; --n)
        ;

    na.shift = clz_nonzero(v[n - 1]);  // Use clz_nonzero() to avoid clang analyzer's warning.
    if (na.shift)
    {
        for (int i = num_words - 1; i > 0; --i)
            vn[i] = (v[i] << na.shift) | (v[i - 1] >> (64 - na.shift));
        vn[0] = v[0] << na.shift;

        un[num_words] = u[num_words - 1] >> (64 - na.shift);
        for (int i = num_words - 1; i > 0; --i)
            un[i] = (u[i] << na.shift) | (u[i - 1] >> (64 - na.shift));
        un[0] = u[0] << na.shift;
    }
    else
    {
        na.numerator_ex = 0;
        na.numerator = numerator;
        na.divisor = denominator;
    }

    // Skip the highest word of numerator if not significant.
    if (un[m] != 0 || un[m - 1] >= vn[n - 1])
        ++m;

    return na;
}

/// Divides arbitrary long unsigned integer by 64-bit unsigned integer (1 word).
/// @param u    The array of a normalized numerator words. It will contain
///             the quotient after execution.
/// @param len  The number of numerator words.
/// @param d    The normalized divisor.
/// @return     The remainder.
inline uint64_t udivrem_by1(uint64_t u[], int len, uint64_t d) noexcept
{
    INTX_REQUIRE(len >= 2);

    const auto reciprocal = reciprocal_2by1(d);

    auto rem = u[len - 1];  // Set the top word as remainder.
    u[len - 1] = 0;         // Reset the word being a part of the result quotient.

    auto it = &u[len - 2];
    do
    {
        std::tie(*it, rem) = udivrem_2by1({rem, *it}, d, reciprocal);
    } while (it-- != &u[0]);

    return rem;
}

/// Divides arbitrary long unsigned integer by 128-bit unsigned integer (2 words).
/// @param u    The array of a normalized numerator words. It will contain the
///             quotient after execution.
/// @param len  The number of numerator words.
/// @param d    The normalized divisor.
/// @return     The remainder.
inline uint128 udivrem_by2(uint64_t u[], int len, uint128 d) noexcept
{
    INTX_REQUIRE(len >= 3);

    const auto reciprocal = reciprocal_3by2(d);

    auto rem = uint128{u[len - 1], u[len - 2]};  // Set the 2 top words as remainder.
    u[len - 1] = u[len - 2] = 0;  // Reset these words being a part of the result quotient.

    auto it = &u[len - 3];
    do
    {
        std::tie(*it, rem) = udivrem_3by2(rem[1], rem[0], *it, d, reciprocal);
    } while (it-- != &u[0]);

    return rem;
}

/// s = x + y.
inline bool add(uint64_t s[], const uint64_t x[], const uint64_t y[], int len) noexcept
{
    // OPT: Add MinLen template parameter and unroll first loop iterations.
    INTX_REQUIRE(len >= 2);

    bool carry = false;
    for (int i = 0; i < len; ++i)
        std::tie(s[i], carry) = add_with_carry(x[i], y[i], carry);
    return carry;
}

/// r = x - multiplier * y.
inline uint64_t submul(
    uint64_t r[], const uint64_t x[], const uint64_t y[], int len, uint64_t multiplier) noexcept
{
    // OPT: Add MinLen template parameter and unroll first loop iterations.
    INTX_REQUIRE(len >= 1);

    uint64_t borrow = 0;
    for (int i = 0; i < len; ++i)
    {
        const auto s = sub_with_carry(x[i], borrow);
        const auto p = umul(y[i], multiplier);
        const auto t = sub_with_carry(s.value, p[0]);
        r[i] = t.value;
        borrow = p[1] + s.carry + t.carry;
    }
    return borrow;
}

inline void udivrem_knuth(
    uint64_t q[], uint64_t u[], int ulen, const uint64_t d[], int dlen) noexcept
{
    INTX_REQUIRE(dlen >= 3);
    INTX_REQUIRE(ulen >= dlen);

    const auto divisor = uint128{d[dlen - 1], d[dlen - 2]};
    const auto reciprocal = reciprocal_3by2(divisor);
    for (int j = ulen - dlen - 1; j >= 0; --j)
    {
        const auto u2 = u[j + dlen];
        const auto u1 = u[j + dlen - 1];
        const auto u0 = u[j + dlen - 2];

        uint64_t qhat;
        if (INTX_UNLIKELY(uint128(u2, u1) == divisor))  // Division overflows.
        {
            qhat = ~uint64_t{0};

            u[j + dlen] = u2 - submul(&u[j], &u[j], d, dlen, qhat);
        }
        else
        {
            uint128 rhat;
            std::tie(qhat, rhat) = udivrem_3by2(u2, u1, u0, divisor, reciprocal);

            bool carry;
            const auto overflow = submul(&u[j], &u[j], d, dlen - 2, qhat);
            std::tie(u[j + dlen - 2], carry) = sub_with_carry(rhat[0], overflow);
            std::tie(u[j + dlen - 1], carry) = sub_with_carry(rhat[1], carry);

            if (INTX_UNLIKELY(carry))
            {
                --qhat;
                u[j + dlen - 1] += divisor[1] + add(&u[j], &u[j], d, dlen - 1);
            }
        }

        q[j] = qhat;  // Store quotient digit.
    }
}

}  // namespace internal

template <unsigned N>
div_result<uint<N>> udivrem(const uint<N>& u, const uint<N>& v) noexcept
{
    auto na = internal::normalize(u, v);

    if (na.num_numerator_words <= na.num_divisor_words)
        return {0, u};

    if (na.num_divisor_words == 1)
    {
        const auto r = internal::udivrem_by1(
            as_words(na.numerator), na.num_numerator_words, as_words(na.divisor)[0]);
        return {na.numerator, r >> na.shift};
    }

    if (na.num_divisor_words == 2)
    {
        const auto d = as_words(na.divisor);
        const auto r =
            internal::udivrem_by2(as_words(na.numerator), na.num_numerator_words, {d[1], d[0]});
        return {na.numerator, r >> na.shift};
    }

    auto un = as_words(na.numerator);  // Will be modified.

    uint<N> q;
    internal::udivrem_knuth(
        as_words(q), &un[0], na.num_numerator_words, as_words(na.divisor), na.num_divisor_words);

    uint<N> r;
    auto rw = as_words(r);
    for (int i = 0; i < na.num_divisor_words - 1; ++i)
        rw[i] = na.shift ? (un[i] >> na.shift) | (un[i + 1] << (64 - na.shift)) : un[i];
    rw[na.num_divisor_words - 1] = un[na.num_divisor_words - 1] >> na.shift;

    return {q, r};
}

template <unsigned N>
inline constexpr div_result<uint<N>> sdivrem(const uint<N>& u, const uint<N>& v) noexcept
{
    const auto sign_mask = uint<N>{1} << (sizeof(u) * 8 - 1);
    auto u_is_neg = (u & sign_mask) != 0;
    auto v_is_neg = (v & sign_mask) != 0;

    auto u_abs = u_is_neg ? -u : u;
    auto v_abs = v_is_neg ? -v : v;

    auto q_is_neg = u_is_neg ^ v_is_neg;

    auto res = udivrem(u_abs, v_abs);

    return {q_is_neg ? -res.quot : res.quot, u_is_neg ? -res.rem : res.rem};
}

template <unsigned N>
inline constexpr uint<N> operator/(const uint<N>& x, const uint<N>& y) noexcept
{
    return udivrem(x, y).quot;
}

template <unsigned N>
inline constexpr uint<N> operator%(const uint<N>& x, const uint<N>& y) noexcept
{
    return udivrem(x, y).rem;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator/=(uint<N>& x, const T& y) noexcept
{
    return x = x / y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator%=(uint<N>& x, const T& y) noexcept
{
    return x = x % y;
}

template <unsigned N>
inline uint<N> bswap(const uint<N>& x) noexcept
{
    static constexpr auto num_words = uint<N>::num_words;
    auto xw = as_words(x);
    uint<N> result;
    auto rw = as_words(result);
    for (size_t i = 0; i < num_words; ++i)
        rw[num_words - 1 - i] = bswap(xw[i]);

    return result;
}


// Support for type conversions for binary operators.

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator+(const uint<N>& x, const T& y) noexcept
{
    return x + uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator+(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) + y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator-(const uint<N>& x, const T& y) noexcept
{
    return x - uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator-(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) - y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator*(const uint<N>& x, const T& y) noexcept
{
    return x * uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator*(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) * y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator/(const uint<N>& x, const T& y) noexcept
{
    return x / uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator/(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) / y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator%(const uint<N>& x, const T& y) noexcept
{
    return x % uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator%(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) % y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator|(const uint<N>& x, const T& y) noexcept
{
    return x | uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator|(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) | y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator&(const uint<N>& x, const T& y) noexcept
{
    return x & uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator&(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) & y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator^(const uint<N>& x, const T& y) noexcept
{
    return x ^ uint<N>(y);
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N> operator^(const T& x, const uint<N>& y) noexcept
{
    return uint<N>(x) ^ y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator|=(uint<N>& x, const T& y) noexcept
{
    return x = x | y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator&=(uint<N>& x, const T& y) noexcept
{
    return x = x & y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator^=(uint<N>& x, const T& y) noexcept
{
    return x = x ^ y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator<<=(uint<N>& x, const T& y) noexcept
{
    return x = x << y;
}

template <unsigned N, typename T,
    typename = typename std::enable_if<std::is_convertible<T, uint<N>>::value>::type>
inline constexpr uint<N>& operator>>=(uint<N>& x, const T& y) noexcept
{
    return x = x >> y;
}


inline uint256 addmod(const uint256& x, const uint256& y, const uint256& mod) noexcept
{
    const auto s = add_with_carry(x, y);
    return (uint512{s.carry, s.value} % mod).lo;
}

inline uint256 mulmod(const uint256& x, const uint256& y, const uint256& mod) noexcept
{
    return (umul(x, y) % mod).lo;
}


inline constexpr uint256 operator"" _u256(const char* s) noexcept
{
    return from_string<uint256>(s);
}

inline constexpr uint512 operator"" _u512(const char* s) noexcept
{
    return from_string<uint512>(s);
}

namespace le  // Conversions to/from LE bytes.
{
template <typename IntT, unsigned M>
inline IntT load(const uint8_t (&bytes)[M]) noexcept
{
    static_assert(M == IntT::num_bits / 8,
        "the size of source bytes must match the size of the destination uint");
    auto x = IntT{};
    std::memcpy(&x, bytes, sizeof(x));
    return x;
}

template <unsigned N>
inline void store(uint8_t (&dst)[N / 8], const intx::uint<N>& x) noexcept
{
    std::memcpy(dst, &x, sizeof(x));
}

}  // namespace le


namespace be  // Conversions to/from BE bytes.
{
/// Loads an uint value from bytes of big-endian order.
/// If the size of bytes is smaller than the result uint, the value is zero-extended.
template <typename IntT, unsigned M>
inline IntT load(const uint8_t (&bytes)[M]) noexcept
{
    static_assert(M <= IntT::num_bits / 8,
        "the size of source bytes must not exceed the size of the destination uint");
    auto x = IntT{};
    std::memcpy(&as_bytes(x)[IntT::num_bits / 8 - M], bytes, M);
    return bswap(x);
}

template <typename IntT, typename T>
inline IntT load(const T& t) noexcept
{
    return load<IntT>(t.bytes);
}

/// Stores an uint value in a bytes array in big-endian order.
template <unsigned N>
inline void store(uint8_t (&dst)[N / 8], const intx::uint<N>& x) noexcept
{
    const auto d = bswap(x);
    std::memcpy(dst, &d, sizeof(d));
}

/// Stores an uint value in .bytes field of type T. The .bytes must be an array of uint8_t
/// of the size matching the size of uint.
template <typename T, unsigned N>
inline T store(const intx::uint<N>& x) noexcept
{
    T r{};
    store(r.bytes, x);
    return r;
}

/// Stores the truncated value of an uint in a bytes array.
/// Only the least significant bytes from big-endian representation of the uint
/// are stored in the result bytes array up to array's size.
template <unsigned M, unsigned N>
inline void trunc(uint8_t (&dst)[M], const intx::uint<N>& x) noexcept
{
    static_assert(M < N / 8, "destination must be smaller than the source value");
    const auto d = bswap(x);
    const auto b = as_bytes(d);
    std::memcpy(dst, &b[sizeof(d) - M], M);
}

/// Stores the truncated value of an uint in the .bytes field of an object of type T.
template <typename T, unsigned N>
inline T trunc(const intx::uint<N>& x) noexcept
{
    T r{};
    trunc(r.bytes, x);
    return r;
}

namespace unsafe
{
/// Loads an uint value from a buffer. The user must make sure
/// that the provided buffer is big enough. Therefore marked "unsafe".
template <typename IntT>
inline IntT load(const uint8_t* bytes) noexcept
{
    auto x = IntT{};
    std::memcpy(&x, bytes, sizeof(x));
    return bswap(x);
}

/// Stores an uint value at the provided pointer in big-endian order. The user must make sure
/// that the provided buffer is big enough to fit the value. Therefore marked "unsafe".
template <unsigned N>
inline void store(uint8_t* dst, const intx::uint<N>& x) noexcept
{
    const auto d = bswap(x);
    std::memcpy(dst, &d, sizeof(d));
}
}  // namespace unsafe

}  // namespace be

}  // namespace intx
