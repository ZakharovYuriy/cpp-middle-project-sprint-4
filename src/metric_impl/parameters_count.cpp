#include "metric_impl/parameters_count.hpp"

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

namespace analyzer::metric::metric_impl {

MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &/*f*/) const {
    // TODO: implement
    return 0;
}

std::string CountParametersMetric::Name() const { return "parameters_count"; }

}  // namespace analyzer::metric::metric_impl
