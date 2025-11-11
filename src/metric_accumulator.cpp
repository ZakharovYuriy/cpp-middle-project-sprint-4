#include "metric_accumulator.hpp"

#include <unistd.h>

#include <algorithm>
#include <any>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

using namespace std;

namespace analyzer::metric_accumulator {

void MetricsAccumulator::AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const {
    auto results =
        metric_results | views::filter([&](const auto &result) { return accumulators.contains(result.metric_name); });
    ranges::for_each(results, [&](const auto &result) { accumulators.at(result.metric_name)->Accumulate(result); });
}

void MetricsAccumulator::ResetAccumulators() {
    ranges::for_each(accumulators, [](auto &pair) {
        if (pair.second)
            pair.second->Reset();
    });
}

}  // namespace analyzer::metric_accumulator
