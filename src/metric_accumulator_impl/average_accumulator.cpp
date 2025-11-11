#include "metric_accumulator_impl/average_accumulator.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace analyzer::metric_accumulator::metric_accumulator_impl {

namespace {

int ExtractIntValue(const metric::MetricResult &metric_result, std::string_view context) {
    if (!std::holds_alternative<int>(metric_result.value))
        throw std::invalid_argument(std::string(context) + " expects integer metric values");
    return std::get<int>(metric_result.value);
}

}  // namespace

void AverageAccumulator::Accumulate(const metric::MetricResult &metric_result) {
    if (is_finalized)
        throw std::logic_error("AverageAccumulator cannot accumulate after finalization");

    sum += ExtractIntValue(metric_result, "AverageAccumulator");
    ++count;
}

void AverageAccumulator::Finalize() {
    if (is_finalized)
        return;

    average = count == 0 ? 0.0 : static_cast<double>(sum) / static_cast<double>(count);
    is_finalized = true;
}

void AverageAccumulator::Reset() {
    sum = 0;
    count = 0;
    average = 0.0;
    is_finalized = false;
}

double AverageAccumulator::Get() const {
    if (!is_finalized)
        throw std::logic_error("AverageAccumulator::Get requires finalized state");
    return average;
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
