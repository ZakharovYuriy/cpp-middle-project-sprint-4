#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyzer {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

auto AnalyseFunctions(const std::vector<std::string> &files,
                      const analyzer::metric::MetricExtractor &metric_extractor) {
    // здесь ваш код
}

auto SplitByClasses(const auto &analysis) {
    // здесь ваш код
}

auto SplitByFiles(const auto &analysis) {
    // здесь ваш код
}

void AccumulateFunctionAnalysis(const auto &analysis,
                                const analyzer::metric_accumulator::MetricsAccumulator &accumulator) {
    // здесь ваш код
}

}  // namespace analyzer
