# Auto-vectorization helpers

![Unit tests](https://github.com/LTLA/auveh/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/LTLA/auveh/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/LTLA/auveh/branch/master/graph/badge.svg?token=OmyPfKEQUm)](https://codecov.io/gh/LTLA/auveh)

## Overview

This library implements a few macros and functions to assist compiler auto-vectorization. 
Most of our loops are simple enough that auto-vectorization does a pretty good job,
effectively giving us a free optimization at `-O2` or higher in recent versions of clang and GCC.
We prefer auto-vectorization over explicit use of intrinsics as the former is more portable,
does not introduce any third-party dependencies,
and takes advantage of the compiler's cost model to determine if vectorization is actually beneficial.

## Ignoring vector dependencies

Most data-parallel loops can be easily auto-vectorized but occasionally some assistance is required.
Consider the following loop for updating a vector at unique indices:

```cpp
void update(
    const std::vector<int>& indices, // unique non-negative indices
    const std::vector<double>& values, // same length as 'indices'
    std::vector<double>& sums
) { 
    const int num_indices = indices.size();
    for (int i = 0; i < num_indices; ++i) {
        sums[indices[i]] += values[i];
    }
}
```

Normally, the compiler cannot assume that iterations of the above loop are independent, as it does not know that `indices` contains only unique values.
However, as we know that the `indices` are unique, we can instruct the compiler to ignore these potential dependencies by adding the `AUVEH_NODEP` macro before the loop.
This expands to a compiler-specific pragma equivalent to GCC's `#pragma GCC ivdep`, which may be enough to induce the compiler to use scatter/gather instructions.
For example, testing the code below on Godbolt indicates that clang will use `vgatherdpd` on recent Intel CPUs.

```cpp
#include "auveh/auveh.hpp"

void update2(
    const std::vector<int>& indices, // unique non-negative indices
    const std::vector<double>& values, // same length as 'indices'
    std::vector<double>& sums
) { 
    // assert(is_unique(indices));
    const int num_indices = indices.size();

    // The major compilers don't mind if our pragma is inline with the 'for',
    // but we'll just use a newline to avoid any potential for confusion. 
    AUVEH_NODEP
    for (int i = 0; i < num_indices; ++i) {
        sums[indices[i]] += values[i];
    }
}
```

The `AUVEH_NODEP` macro can still be useful for simple loops that are obviously vectorizable.
In the example below, we can guarantee that `left` and `right` are different vectors from `output`.
Adding the macro allows both clang and GCC to omit aliasing checks for a smaller, more efficient binary.

```cpp
void dot(
    const std::vector<double>& left,
    const std::vector<double>& right,
    std::vector<double>& output // not an alias for 'left' or 'right'.
) { 
    // assert(&left != &output);
    // assert(&right != &output);
    const std::size_t n = left.size();
    AUVEH_NODEP for (std::size_t i = 0; i < n; ++i) {
        output[i] += left[i] * right[i];
    }
}
```

## Specifying the vector width

On occasion, we might need to know the native vector width, e.g., 128 bits for SSE, 256 for AVX, and so on.
We can get this by calling `vector_width()` with the type of interest:

```cpp
constexpr int native_width = auveh::vector_width<int>();
```

For example, when processing data in blocks, we might choose a block size that is a multiple of the vector width.
This improves the performance of auto-vectorized code by reducing the number of entries into an epilogue loop. 

```cpp
// Blocked matrix multiplication between a column-major LHS,
// a row-major RHS and a row-major output matrix.
void blocked_mult_with_right_row_to_output_row(
    const std::size_t NR,
    const std::size_t NC,
    const std::vector<std::vector<float> >& matrix,
    const std::size_t NRHS,
    const std::vector<std::vector<float> >& rhs,
    std::vector<std::vector<float> >& product,
    std::size_t outer_block_size
) {
    std::size_t c = 0;
    while (c < NC) { 
        const std::size_t cend = c + std::min(outer_block_size, NC - c);
        std::size_t r = 0;
        while (r < NR) {
            const std::size_t rend = r + std::min(outer_block_size, NR - r);
            std::size_t h = 0;
            while (h < NRHS) {
                constexpr std::size_t inner_block_size = 8 * auveh::vector_width<float>();
                const std::size_t hend = h + std::min(inner_block_size, NRHS - h);
                for (auto ccopy = c; ccopy < cend; ++ccopy) {
                    const auto& matcol = matrix[ccopy];
                    const auto& rightrow = rhs[ccopy];
                    for (auto rcopy = r; rcopy < rend; ++rcopy) {
                        const auto mult = matcol[rcopy];
                        auto& outrow = product[rcopy];
                        for (auto hcopy = h; hcopy < hend; ++hcopy) {
                            outrow[hcopy] += mult * rightrow[hcopy];
                        }
                    }
                }

                h = hend;
            }
            r = rend;
        }
        c = cend;
    }
}
```

Another application is to use the native vector width to define the number of accumulators for a summation.
This strongly encourages the auto-vectorizer to use the appropriate vector instructions,
albeit at the cost of returning slightly different outputs for machines with different register vector widths.

```cpp
void sum(const std::size_t num, const double* input) {
    constexpr std::size_t num_acc = auveh::vector_width<double>();
    const std::size_t iters = num / num_acc;
    const std::size_t remainders = num % num_acc;
    double output = 0;

    if (iters >= 1) {
        std::array<double, num_acc> sums;
        std::copy_n(input, num_acc, sums.begin());
        for (std::size_t i = 1; i < iters; ++i) {
            // compiler replaces this inner loop with a single vector instruction.
            for (std::size_t a = 0; a < num_acc; ++a) {
                sums[a] += input[a + i * num_acc];
            }
        }
        for (std::size_t a = 0; a < num_acc; ++a) {
            output += sums[a];
        }
    }

    for (std::size_t a = 0; a < remainders; ++a) {
        output += input[a + iters * num_acc]; 
    }
    return output;
}
```

## Building projects 

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  auveh
  GIT_REPOSITORY https://github.com/LTLA/auveh
  GIT_TAG master # or any version of interest 
)

FetchContent_MakeAvailable(auveh)
```

Then you can link to **auveh** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe auveh)

# For libaries
target_link_libraries(mylib INTERFACE auveh)
```

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DAUVEH_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(ltla_auveh CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE ltla::auveh)
```

### Manual

If you're not using CMake, the simple approach is to just copy the files in the `include/` subdirectory - 
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
