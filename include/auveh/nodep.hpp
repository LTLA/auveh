#ifndef AUVEH_NODEP_HPP
#define AUVEH_NODEP_HPP

/**
 * @file nodep.hpp
 * @brief No vector dependencies in a loop.
 */

#ifdef NDEBUG
#ifndef AUVEH_NODEP
#if defined(__clang__)
#define AUVEH_NODEP _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define AUVEH_NODEP _Pragma("GCC ivdep")
#elif defined(__INTEL_LLVM_COMPILER)
#define AUVEH_NODEP _Pragma("ivdep")
#elif defined(_MSC_VER)
#define AUVEH_NODEP _Pragma("loop(ivdep)")
#else
#define AUVEH_NODEP
#endif
#endif
#else
/**
 * Assert that the following `for` loop has no vector dependencies.
 *
 * This macro should be placed just above a `for` statement.
 * It inserts appropriate pragmas to assert that the loop body has no dependencies between iterations, which creates more opportunities for auto-vectorization of the loop.
 * Even for trivial loops, it allows the compiler to omit aliasing checks for a more compact binary.
 *
 * When `NDEBUG` is defined, the `AUVEH_NODEP` macro has a compiler-specific definition:
 *
 * - clang: `#pragma loop vectorize(assume_safety)`.
 * - GCC: `#pragma GCC ivdep`
 * - ICC: `#pragma ivdep`
 * - MSVC: `#pragma loop(ivdep)`
 * 
 * For other compilers, it is left empty.
 *
 * When `NDEBUG` is not defined, the `AUVEH_NODEP` macro is left empty.
 * This allows the loop body to contain `assert()` statements in debug builds, which would otherwise prevent autovectorization.
 *
 * If `AUVEH_NODEP` is already defined before including `nodep.hpp`, the existing definition will be preserved.
 * This is occasionally useful to disable all vectorization instructions, e.g., for trapping math (see below).
 *
 * @section dependencies Vector dependencies
 *
 * The exact nature of the dependencies to be ignored will differ across compilers. 
 * ICC's pragma will [not ignore proven dependencies](https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2025-0/ivdep.html),
 * while the pragmas for [clang](https://discourse.llvm.org/t/llvm-rfc-addition-support-of-new-vectorization-pragmas-in-llvm/52785/3)
 * and [GCC](https://gcc.gnu.org/onlinedocs/gcc/Loop-Specific-Pragmas.html) assert that there are no dependencies at all.
 *
 * For correctness on all platforms, the `AUVEH_NODEP` macro should be used conservatively, i.e., only for loops where the developer knows that there are no dependencies. 
 * A loop is suitable for `AUVEH_NODEP` if we are able to execute its body for different iterations:
 * 
 * - In parallel without race conditions.
 * - In any order without affecting the result.
 *
 * The latter implies that there are no changes in control flow within the body that might cause the loop to prematurely exit.
 * This is usually obvious, e.g., no `break`, `return` or `throw` within the body,
 * but it also asserts that signals will not be raised from floating-point exception traps, out-of-bounds casts to signed integers, etc.
 * We believe that this assertion is reasonable in the vast majority of applications using the default compiler settings.
 * Nonetheless, if strictly conforming behavior is required, developers can manually define `AUVEH_NODEP` to a no-op. 
 *
 * @section openmp-simd OpenMP SIMD
 *
 * OpenMP SIMD also provides a pragma with similar behavior (`#pragma omp simd`). 
 * While this works and is portable, it is often subject to a more heavy-handed interpretation by compilers.
 * Upon seeing `#pragma omp simd`, [GCC](https://developers.redhat.com/articles/2023/12/08/vectorization-optimization-gcc) will forcibly vectorize the loop,
 * even if doing so would decrease performance according to its cost model.
 * [MSVC](https://devblogs.microsoft.com/cppblog/simd-extension-to-c-openmp-in-visual-studio/) goes further and enables fast floating-point inside the loop, which is not generally desirable.
 */
#define AUVEH_NODEP
#endif

#endif
