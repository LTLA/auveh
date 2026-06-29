# Auto-vectorization helpers

## Overview

This library implements a few macros and functions to help compiler auto-vectorization.
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
    AUVEH_NODEP for (int i = 0; i < num_indices; ++i) {
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
