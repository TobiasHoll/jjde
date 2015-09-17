#ifndef JJDE_BYTES_HPP
#define JJDE_BYTES_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <type_traits>
#include <vector>

namespace jjde {

/* Read bytes from the stream */

template <std::size_t N>
std::array<unsigned char, N> extract(std::ifstream & stream) {
    std::array<unsigned char, N> data;
    for (std::size_t i = 0; i < N; ++i) {
        data[i] = (unsigned char) stream.get();
    }
    return data;
}

std::vector<unsigned char> extract(std::ifstream & stream, std::size_t N) {
    std::vector<unsigned char> data(N);
    for (std::size_t i = 0; i < N; ++i) {
        data[i] = (unsigned char) stream.get();
    }
    return data;
}

/* Parse the bytes into a value */

/* Parse unsigned integers */
template <typename T, std::size_t N>
typename std::enable_if<std::is_unsigned<T>::value && std::is_integral<T>::value, T>::type parse(std::array<unsigned char, N> const& raw) {
    T value = 0;
    for (std::size_t i = 0; i < N; ++i) {
        value <<= 8;
        value |= raw[i];
    }
    return value;
}

/* Parse signed integers */
template <typename T, std::size_t N>
typename std::enable_if<std::is_signed<T>::value && std::is_integral<T>::value, T>::type parse(std::array<unsigned char, N> const& raw) {
    T value = raw[0] & 0x7F; // Skip sign bit
    for (std::size_t i = 1; i < N; ++i) {
        value <<= 8;
        value |= raw[i];
    }
    if (raw[0] & 0x80) {
        // Sign bit is set.
        value -= ((T) 1) << (N * 8 - 1);
    }
    return value;
}

/* Parse floating point numbers (IEEE 754, 32-bit and 64-bit) */

namespace detail {

template <int EXP, int MAN, typename T, typename Exponent, typename Mantissa>
T construct_floating_point(int8_t sign, Exponent exponent, Mantissa mantissa) {
    bool denormalized = false;
    if (exponent == (1 << (EXP + 1)) - 1) {
        if (mantissa == 0) return (T) sign * std::numeric_limits<T>::infinity();
        else return (T) sign * std::numeric_limits<T>::quiet_NaN();
    } else if (exponent == 0) {
        if (mantissa == 0) return (T) 0; // Zero value loses its sign, which is entirely irrelevant (usually)
        else denormalized = true;
    }
    T converted = (T) (denormalized ? 0 : std::pow(2, exponent));
    for (int index = 1; index <= MAN; ++index) {
        if (mantissa & (((Mantissa) 1) << (MAN - index))) converted += std::pow(2, exponent - index);
    }
    return (T) sign * converted;
}

}

/* 32-bit (float) */
template <typename T, std::size_t N>
typename std::enable_if<std::is_floating_point<T>::value && N == 4, T>::type parse(std::array<unsigned char, N> const& raw) {
    int8_t sign = (raw[0] & 0x80) ? -1 : 1; // 1 sign bit
    int16_t exponent = (((raw[0] & 0x7F) << 1) | ((raw[1] & 0x80) >> 7)) - 0x7F; // 8 exponent bits
    uint32_t mantissa = ((raw[1] & 0x7F) << 16) | (raw[2] << 8) | raw[3]; // 23 mantissa bits

    return detail::construct_floating_point<8, 23, T>(sign, exponent, mantissa);
}
/* 64-bit (double) */
template <typename T, std::size_t N>
typename std::enable_if<std::is_floating_point<T>::value && N == 8, T>::type parse(std::array<unsigned char, N> const& raw) {
    int8_t sign = (raw[0] & 0x80) ? -1 : 1; // 1 sign bit
    int16_t exponent = ((((uint16_t) raw[0] & 0x7F) << 4) | (((uint16_t) raw[1] & 0xF0) >> 4)) - 0x3FF; // 11 exponent bits
    uint64_t mantissa = (((uint64_t) raw[1] & 0x0F) << 48) | ((uint64_t) raw[2] << 40) | ((uint64_t) raw[3] << 32) | ((uint64_t) raw[4] << 24) | ((uint64_t) raw[5] << 16) | ((uint64_t) raw[6] << 8) | ((uint64_t) raw[7]); // 52 mantissa bits

    return detail::construct_floating_point<11, 52, T>(sign, exponent, mantissa);
}

/* Convert from variable-length vectors */
template <std::size_t N>
std::array<unsigned char, N> convert(std::vector<unsigned char> const& raw, std::size_t start=0) {
    std::array<unsigned char, N> array;
    for (std::size_t index = 0; index < N; ++index) {
        array[index] = raw[start + index];
    }
    return array;
}

template <std::size_t N, typename Iterator>
typename std::enable_if<!std::is_same<Iterator, std::vector<unsigned char>>::value, std::array<unsigned char, N>>::type convert(Iterator & it) {
    std::array<unsigned char, N> array;
    for (std::size_t index = 0; index < N; ++index) {
        array[index] = *it++;
    }
    return array;
}

/* Unsafe memory operations */

template <typename T, std::size_t N>
void deserialize(std::array<unsigned char, N> && raw, T *pointer) {
	memcpy(pointer, raw.data(), N);
}

template <typename T>
void deserialize(std::vector<unsigned char> && raw, T *pointer) {
	memcpy(pointer, raw.data(), raw.size());
}

}

#endif // JJDE_BYTES_HPP
