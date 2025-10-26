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
    // здесь ваш код
}

void MetricsAccumulator::ResetAccumulators() {
    // здесь ваш код
}

}  // namespace analyzer::metric_accumulator
