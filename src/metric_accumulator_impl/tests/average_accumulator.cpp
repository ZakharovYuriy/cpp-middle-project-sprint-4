#include "metric_accumulator_impl/average_accumulator.hpp"

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

TEST(AverageAccumulatorTest, AccumulatesSingleValue) {
    AverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(10));
    accumulator.Finalize();

    EXPECT_DOUBLE_EQ(accumulator.Get(), 10.0);
}

TEST(AverageAccumulatorTest, CalculatesAverageOfMultipleValues) {
    AverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(10));
    accumulator.Accumulate(MakeMetricResult(20));
    accumulator.Accumulate(MakeMetricResult(30));
    accumulator.Finalize();

    EXPECT_DOUBLE_EQ(accumulator.Get(), 20.0);
}

TEST(AverageAccumulatorTest, FinalizeWithoutValuesYieldsZero) {
    AverageAccumulator accumulator;
    accumulator.Finalize();

    EXPECT_DOUBLE_EQ(accumulator.Get(), 0.0);
}

TEST(AverageAccumulatorTest, ResetClearsState) {
    AverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(5));
    accumulator.Finalize();

    EXPECT_DOUBLE_EQ(accumulator.Get(), 5.0);

    accumulator.Reset();
    accumulator.Accumulate(MakeMetricResult(10));
    accumulator.Accumulate(MakeMetricResult(20));
    accumulator.Finalize();

    EXPECT_DOUBLE_EQ(accumulator.Get(), 15.0);
}

TEST(AverageAccumulatorTest, AccumulateAfterFinalizeThrows) {
    AverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(1));
    accumulator.Finalize();

    EXPECT_THROW(accumulator.Accumulate(MakeMetricResult(2)), std::logic_error);
}

TEST(AverageAccumulatorTest, GetBeforeFinalizeThrows) {
    AverageAccumulator accumulator;
    accumulator.Accumulate(MakeMetricResult(42));

    EXPECT_THROW(accumulator.Get(), std::logic_error);
}

TEST(AverageAccumulatorTest, RejectsNonIntegerValues) {
    AverageAccumulator accumulator;
    metric::MetricResult wrong_value{.metric_name = "metric", .value = std::string("not an int")};

    EXPECT_THROW(accumulator.Accumulate(wrong_value), std::invalid_argument);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
