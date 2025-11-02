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

namespace analyzer::metric_accumulator {

void MetricsAccumulator::AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const {
    for (const auto &result : metric_results) {
        auto it = accumulators.find(result.metric_name);
        if (it == accumulators.end())
            continue;
        it->second->Accumulate(result);
    }
}

void MetricsAccumulator::ResetAccumulators() {
    for (auto &[name, accumulator] : accumulators) {
        if (accumulator)
            accumulator->Reset();
    }
}

}  // namespace analyzer::metric_accumulator
