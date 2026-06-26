#ifndef AUVEH_WIDTH_HPP
#define AUVEH_WIDTH_HPP

/**
 * @file width.hpp
 * @brief Detect SIMD vector width.
 */

#if defined(__AVX512F__)
#define AUVEH_WIDTH 512
#elif defined(__AVX__)
#define AUVEH_WIDTH 256
#elif defined(__SSE__)
#define AUVEH_WIDTH 128
#elif defined(__ARM_NEON)
#define AUVEH_WIDTH 128
#elif defined(__ARM_FEATURE_SVE)
// We will use the lowest available unit for the variable-length registers.
#define AUVEH_WIDTH 128
#else
/**
 * Detect the SIMD vector width in the current environment.
 *
 * Widths are reported in bits. Currently, we consider:
 *
 * - 128 for all SSE versions.
 * - 256 for AVX and AVX2.
 * - 512 for AVX-512.
 * - 128 for Arm Neon.
 * - 128 for Arm SVE.
 *   Technically SVE is variable length, but we will consider the smallest possible unit as our common denominator.
 *
 * If no vector capability is available, the register size is set to zero.
 */
#define AUVEH_WIDTH 0
#endif

#endif
