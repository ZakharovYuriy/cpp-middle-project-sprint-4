#include "metric_accumulator_impl/categorical_accumulator.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

namespace {

metric::MetricResult MakeCategoryResult(std::string_view category) {
    return metric::MetricResult{.metric_name = "metric", .value = std::string(category)};
}

metric::MetricResult MakeNumericResult(int value) {
    return metric::MetricResult{.metric_name = "metric", .value = value};
}

}  // namespace

TEST(CategoricalAccumulatorTest, CountsSingleCategory) {
    CategoricalAccumulator accumulator;
    accumulator.Accumulate(MakeCategoryResult("alpha"));
    accumulator.Finalize();

    const auto &freq = accumulator.Get();
    ASSERT_EQ(freq.size(), 1u);
    EXPECT_EQ(freq.at("alpha"), 1);
}

TEST(CategoricalAccumulatorTest, CountsMultipleCategories) {
    CategoricalAccumulator accumulator;
    accumulator.Accumulate(MakeCategoryResult("alpha"));
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Accumulate(MakeCategoryResult("alpha"));
    accumulator.Accumulate(MakeCategoryResult("gamma"));
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Finalize();

    const auto &freq = accumulator.Get();
    EXPECT_EQ(freq.at("alpha"), 2);
    EXPECT_EQ(freq.at("beta"), 3);
    EXPECT_EQ(freq.at("gamma"), 1);
}

TEST(CategoricalAccumulatorTest, FinalizeWithoutValuesProducesEmptyMap) {
    CategoricalAccumulator accumulator;
    accumulator.Finalize();

    EXPECT_TRUE(accumulator.Get().empty());
}

TEST(CategoricalAccumulatorTest, ResetClearsState) {
    CategoricalAccumulator accumulator;
    accumulator.Accumulate(MakeCategoryResult("alpha"));
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Finalize();

    EXPECT_EQ(accumulator.Get().size(), 2u);

    accumulator.Reset();
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Accumulate(MakeCategoryResult("beta"));
    accumulator.Finalize();

    const auto &freq = accumulator.Get();
    ASSERT_EQ(freq.size(), 1u);
    EXPECT_EQ(freq.at("beta"), 2);
}

TEST(CategoricalAccumulatorTest, RejectsNonStringValues) {
    CategoricalAccumulator accumulator;

    EXPECT_THROW(accumulator.Accumulate(MakeNumericResult(42)), std::invalid_argument);
}

TEST(CategoricalAccumulatorTest, AccumulateAfterFinalizeThrows) {
    CategoricalAccumulator accumulator;
    accumulator.Finalize();

    EXPECT_THROW(accumulator.Accumulate(MakeCategoryResult("alpha")), std::logic_error);
}

TEST(CategoricalAccumulatorTest, GetBeforeFinalizeThrows) {
    CategoricalAccumulator accumulator;
    accumulator.Accumulate(MakeCategoryResult("alpha"));

    EXPECT_THROW(accumulator.Get(), std::logic_error);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
