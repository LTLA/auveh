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
 * This macro should be placed just above in front of a `for` statement.
 * It inserts appropriate pragmas to instruct the compiler to ignore dependencies in the loop body.
 * This creates more opportunities for auto-vectorization of the loop if assumed dependencies were present.
 * Even for trivial loops, it allows the compiler to omit aliasing checks for a more compact and efficient binary.
 *
 * This macro has the following definitions for each compiler:
 *
 * - clang: `#pragma loop vectorize(assume_safety)`.
 * - GCC: `#pragma GCC ivdep`
 * - ICC: `#pragma ivdep`
 * - MSVC: `#pragma loop(ivdep)`
 * 
 * Otherwise, it is left empty.
 * Note that the dependencies to be ignored will differ across compilers. 
 * ICC's pragma will not ignore proven dependencies (https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2025-0/ivdep.html),
 * while clang's pragma asserts that there are [no dependencies at all](https://discourse.llvm.org/t/llvm-rfc-addition-support-of-new-vectorization-pragmas-in-llvm/52785/3).
 * Thus, for correctness in all platforms, this macro should be used conservatively, i.e., only for loops where there are no dependencies at all.
 * 
 * We note that OpenMP SIMD also provides a pragma with similar behavior (`#pragma omp simd`). 
 * While this works and is portable, it is often subject to a more heavy-handed interpretation by compilers.
 * Upon seeing `#pragma omp simd`, [GCC](https://developers.redhat.com/articles/2023/12/08/vectorization-optimization-gcc) will forcibly vectorize the loop,
 * even if doing so would decrease performance according to its cost model.
 * [MSVC](https://devblogs.microsoft.com/cppblog/simd-extension-to-c-openmp-in-visual-studio/) goes further and enables fast floating-point inside the loop, which is not generally desirable.
 */
#define AUVEH_IVDEP
#endif

#endif
