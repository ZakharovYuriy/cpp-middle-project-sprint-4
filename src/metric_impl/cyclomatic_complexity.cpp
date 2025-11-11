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
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

using namespace std;

namespace analyzer::metric::metric_impl {
static const std::unordered_set<std::string_view> kCyclomaticNodes = {
    "if_statement",        "elif_clause", "while_statement",  "for_statement",         "except_clause",
    "except_group_clause", "case_clause", "assert_statement", "conditional_expression"};

MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &f) const {
    constexpr auto delim{"\n"sv};
    return static_cast<int>(ranges::distance(views::split(f.ast, delim) | views::filter([](auto &&line) {
                                                 return ranges::any_of(kCyclomaticNodes, [&line](auto &&node) {
                                                     return ranges::contains_subrange(line, node);
                                                 });
                                             })));
}

std::string CyclomaticComplexityMetric::Name() const { return "cyclomatic_complexity"; }

}  // namespace analyzer::metric::metric_impl
