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
 * while clang's pragma asserts that there are [no dependencies at all](https://discourse.llvm.org/t/llvm-rfc-addition-support-of-new-vectorization-pragmas-in-llvm/52785/3).
 * This results in some interesting differences between compilers when `AUVEH_NODEP` is added to a loop with dependencies,
 * e.g., clang will incorrectly vectorize it while GCC will just generate scalar code.
 *
 * For portability, the `AUVEH_NODEP` macro should be treated as the developer's assertion that there are no dependencies of any kind (proven or assumed).
 * This is the most conservative interpretation that satisfies all compilers' conditions for their corresponding pragmas.
 * Under this policy, a loop is only suitable for `AUVEH_NODEP` if:
 *
 * - Each loop iteration writes to memory addresses that are not read/written by any other iteration.
 *   Note that this precludes functions that set global variables like `errno`, e.g., from `<cmath>`.
 * - The total number of loop iterations is not changed by any iteration.
 *   This is a "dependency" between iterations in the sense that one iteration could cause later iterations to not run at all.
 *   Thus, there cannot be any changes in control flow inside the loop body, i.e., no `break`, `return` or `throw`.
 *
 * The second condition precludes many functions that might throw, including those that might allocate memory.
 * It also assumes that signals will not be raised from floating-point exception traps or out-of-bounds casts to signed integers.
 * We believe that this assumption is reasonable in the vast majority of applications where trapping is not performed.
 * Nonetheless, if strict conformance to the standard is required, developers can manually define `AUVEH_NODEP` to a no-op.
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
