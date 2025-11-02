#include "metric_impl/cyclomatic_complexity.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyzer::metric::metric_impl {

MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &/*f*/) const {
    // TODO: implement
    return 0;
}

std::string CyclomaticComplexityMetric::Name() const { return "cyclomatic_complexity"; }

}  // namespace analyzer::metric::metric_impl
