#include "metric.hpp"

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

#include "function.hpp"

namespace analyzer::metric {

void MetricExtractor::RegisterMetric(std::unique_ptr<IMetric> metric) {
    // здесь ваш код
}

MetricResults MetricExtractor::Get(const function::Function &func) const {
    // здесь ваш код
    return {};
}

}  // namespace analyzer::metric
