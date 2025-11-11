#include "metric_accumulator_impl/categorical_accumulator.hpp"

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

const std::string &ExtractStringValue(const metric::MetricResult &metric_result, std::string_view context) {
    if (!std::holds_alternative<std::string>(metric_result.value))
        throw std::invalid_argument(std::string(context) + " expects string metric values");
    return std::get<std::string>(metric_result.value);
}

}  // namespace

void CategoricalAccumulator::Accumulate(const metric::MetricResult &metric_result) {
    if (is_finalized)
        throw std::logic_error("CategoricalAccumulator cannot accumulate after finalization");

    const auto &category = ExtractStringValue(metric_result, "CategoricalAccumulator");
    ++categories_freq[category];
}

void CategoricalAccumulator::Finalize() {
    if (is_finalized)
        return;
    is_finalized = true;
}

void CategoricalAccumulator::Reset() {
    categories_freq.clear();
    is_finalized = false;
}

const std::unordered_map<std::string, int> &CategoricalAccumulator::Get() const {
    if (!is_finalized)
        throw std::logic_error("CategoricalAccumulator::Get requires finalized state");
    return categories_freq;
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
