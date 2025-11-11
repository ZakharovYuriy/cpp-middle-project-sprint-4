#include "metric_accumulator_impl/sum_average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <string>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

namespace {

metric::MetricResult MakeMetricResult(int value) {
    return metric::MetricResult{.metric_name = "metric", .value = value};
}

}  // namespace

TEST(SumAverageAccumulatorTest, TracksSumAndAverage) {
    SumAverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(4));
    accumulator.Accumulate(MakeMetricResult(6));
    accumulator.Accumulate(MakeMetricResult(10));
    accumulator.Finalize();

    const auto result = accumulator.Get();
    EXPECT_EQ(result.sum, 20);
    EXPECT_DOUBLE_EQ(result.average, 20.0 / 3.0);
}

TEST(SumAverageAccumulatorTest, FinalizeWithoutValuesReturnsZeroes) {
    SumAverageAccumulator accumulator;
    accumulator.Finalize();

    const auto result = accumulator.Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_DOUBLE_EQ(result.average, 0.0);
}

TEST(SumAverageAccumulatorTest, ResetClearsAggregates) {
    SumAverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(5));
    accumulator.Accumulate(MakeMetricResult(7));
    accumulator.Finalize();

    auto result = accumulator.Get();
    EXPECT_EQ(result.sum, 12);
    EXPECT_DOUBLE_EQ(result.average, 6.0);

    accumulator.Reset();
    accumulator.Accumulate(MakeMetricResult(2));
    accumulator.Accumulate(MakeMetricResult(8));
    accumulator.Finalize();

    result = accumulator.Get();
    EXPECT_EQ(result.sum, 10);
    EXPECT_DOUBLE_EQ(result.average, 5.0);
}

TEST(SumAverageAccumulatorTest, AccumulateAfterFinalizeThrows) {
    SumAverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(1));
    accumulator.Finalize();

    EXPECT_THROW(accumulator.Accumulate(MakeMetricResult(2)), std::logic_error);
}

TEST(SumAverageAccumulatorTest, GetBeforeFinalizeThrows) {
    SumAverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(10));

    EXPECT_THROW(accumulator.Get(), std::logic_error);
}

TEST(SumAverageAccumulatorTest, RejectsNonIntegerValues) {
    SumAverageAccumulator accumulator;
    metric::MetricResult wrong_result{.metric_name = "metric", .value = std::string("NaN")};

    EXPECT_THROW(accumulator.Accumulate(wrong_result), std::invalid_argument);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
