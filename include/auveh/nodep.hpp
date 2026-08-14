#ifndef AUVEH_NODEP_HPP
#define AUVEH_NODEP_HPP

/**
 * @file nodep.hpp
 * @brief No vector dependencies in a loop.
 */

#ifdef NDEBUG
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
#else
/**
 * Assert that the following `for` loop has no vector dependencies.
 *
 * This macro should be placed just above in front of a `for` statement.
 * It inserts appropriate pragmas to instruct the compiler to ignore dependencies in the loop body.
 * This creates more opportunities for auto-vectorization of the loop if assumed dependencies were present.
 * Even for trivial loops, it allows the compiler to omit aliasing checks for a more compact and efficient binary.
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
 * The exact nature of the dependencies to be ignored will differ across compilers. 
 * ICC's pragma will [not ignore proven dependencies](https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2025-0/ivdep.html),
 * while clang's pragma asserts that there are [no dependencies at all](https://discourse.llvm.org/t/llvm-rfc-addition-support-of-new-vectorization-pragmas-in-llvm/52785/3).
 * Thus, for correctness on all platforms, this macro should be used conservatively, i.e., only for loops where there are no dependencies at all.
 *
 * When `NDEBUG` is not defined, the `AUVEH_NODEP` macro is left empty.
 * This allows the loop body to contain `assert()` statements in debug builds, which would otherwise preclude autovectorization.
 *
 * @section loop-operations Loop operations
 *
 * The loop body should only contain array accesses and arithmetic operations.
 * Standard library functions should generally be avoided as many of them have side effects involving global variables.
 * For example, many `<cmath>` functions will set `errno`, which precludes vectorization unless the compiler is explicitly instructed to ignore `errno`.
 * (Indeed, adding `AUVEH_NODEP` to a loop with a function call like `std::sqrt()` will cause clang to emit a warning about vectorization failure.)
 *
 * The loop body should refrain from examining floating point exceptions.
 * Doing so will probably prohibit autovectorization, but even if it didn't, exceptions will not be set independently for each loop iteration after vectorization.
 * Rather, exceptions should be tested after the loop has completed. 
 * The bits are sticky so any exception in any iteration will persist after the loop has finished. 
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
