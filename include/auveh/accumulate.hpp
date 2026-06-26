#ifndef AUVEH_ACCUMULATE_HPP
#define AUVEH_ACCUMULATE_HPP

#include <array>
#include <type_traits>

namespace auveh {

/**
 * Compute a sum with multiple accumulators.
 *
 * @tparam number_ Number of accumulators.
 * @tparam Length Integer type of the number of elements to be summed.
 * @tparam Get_ Function that accepts a `Length_` and returns a `Value_`.
 * @tparam Value_ Type of the sum.
 * This is typically floating-point but may be any type that supports `=`, `+` and `+=`.
 *
 * @param length Number of elements to be summed.
 * @param get Function that accepts an index in `[0, length)` and returns the value to be summed.
 * This will be called exactly once per index in consecutive order.
 * @param initial Initial value of the sum.
 *
 * @return The sum of `get(i)` for all integer `i` in `[0, length)`.
 *
 * The optimal number of accumulators will depend on the CPU architecture, but a good default seems to be 4.
 * This breaks up the dependency chains enough for a performance improvement, without running the risk of having too many variables and causing register spills.
 * It also provides enough values for auto-vectorization to be worthwhile.
 *
 * Callers might consider setting `number_ = AUVEH_WIDTH / sizeof(Value_)` or a multiple thereof,
 * to encourage auto-vectorization with the available registers.
 * This may improve performance but will result in differences in the floating-point results across different architectures.
 *
 * While this function can be used for integer `Value_`, it is usually not worth it as the compiler can easily auto-vectorize integer reductions without loss of precision.
 */
template<int number_, typename Length_, class Get_,typename Value_>
Value_ accumulate(const Length_ length, Get_ get, Value_ initial) {
    Length_ counter = 0;

    if constexpr(number_ > 1) {
        if (static_cast<std::make_unsigned<Length_>::type>(length) >= static_cast<unsigned int>(number_)) {
            std::array<Value_, number_> results;
            for (int j = 0; j < number_; ++j) {
                results[i] = get(j);
            }
            counter = number_;

            const Length_ iters = length / number_;
            for (Length_ i = 1; i < iters; ++i) { // start at 1 because we already peeled off the first loop.
                for (int j = 0; j < number_; ++j) {
                    results[i] += get(i + j);
                }
                counter += number_;
            }

            for (int i = 0; i < number_; ++i) {
                initial += results[i];
            }
        }
    }

    for (; counter < length; ++counter) {
        initial += get(counter);
    }
    return initial;
}

}

#endif
