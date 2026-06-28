#ifndef AUVEH_WIDTH_HPP
#define AUVEH_WIDTH_HPP

#if __has_include(<simd>)
#include <simd>
#elif __has_include(<experimental/simd>)
#include <experimental/simd>
#else
#include <algorithm>
#include <type_traits>
#endif

/**
 * @file width.hpp
 * @brief Detect SIMD vector width.
 */

namespace auveh {

/**
 * Get the vector width in terms of the number of elements of the specified `Type_`.
 * In C++26, this will use the native size of `std::simd::basic_vec`, otherwise it will try to infer the vector width from various compiler macros.
 * This is occasionally useful to encourage autovectorization when processing a fixed number of elements, e.g., in blocking schemes.
 *
 * @tparam Type_ Type of each vector element.
 * This is typically a numeric type and its size should be a power of 2.
 *
 * @return Vector width in terms of the number of elements of type `Type_`.
 * This is guaranteed to be positive and is set to 1 if no vector capability is available or if `Type_` is larger than the SIMD vector width.
 */
template<typename Type_>
constexpr int width() {
#if __has_include(<simd>)
    return std::simd::basic_vec<Type_>::size();
#elif __has_include(<experimental/simd>)
    return std::experimental::simd<Type_>::size();
#else
#if defined(__AVX512F__)
    constexpr int w = 512;
#elif defined(__AVX2__)
    constexpr int w = 256;
#elif defined(__AVX__)
    constexpr int w = [&](){
        if constexpr(std::is_integral<Type_>::value) {
            return 128;
        } else {
            return 256;
        }
    }();
#elif defined(__SSE__)
    constexpr int w = 128;
#elif defined(__ARM_NEON)
    constexpr int w = 128;
#elif defined(__ARM_FEATURE_SVE)
    // Technically SVE is variable length, but we will consider the smallest possible unit as our common denominator.
    // Not sure how many machines will actually be created with higher lengths, anyway. 
    constexpr int w = 128; 
#else
    constexpr int w = 0;
#endif
    return std::max(w, 1);
#endif
}

}

#endif
