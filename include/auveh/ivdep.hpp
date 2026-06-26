#ifndef AUVEH_IVDEP_HPP
#define AUVEH_IVDEP_HPP

/**
 * @file ivdep.hpp
 * @brief Ignore vector dependencies in a loop.
 */

#if defined(__clang__)
#define AUVEH_IVDEP _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define AUVEH_IVDEP _Pragma("GCC ivdep")
#elif defined(__INTEL_LLVM_COMPILER)
#define AUVEH_IVDEP _Pragma("ivdep")
#elif defined(_MSC_VER)
#define AUVEH_IVDEP _Pragma("loop(ivdep)")
#else
/**
 * Indicate that the following `for` loop has no vector dependencies.
 *
 * This macro should be placed just before or in front of a `for` statement.
 * It inserts the appropriate pragmas to instruct the compiler to ignore dependencies in the loop body.
 * This creates more opportunities for auto-vectorization of the loop body.
 *
 * Macros are currently defined for each of the major compilers (clang, GCC, ICC and MSVC).
 * While OpenMP SIMD is more portable, it forces vectorization regardless of the compiler's cost model.
 * MSVC also enables fast-math with OpenMP SIMD, which is not desirable.
 */
#define AUVEH_IVDEP
#endif

#endif
