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
#include <unordered_map>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyzer {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

using FunctionAnalysisEntry = std::pair<function::Function, metric::MetricResults>;
using FunctionAnalysis = std::vector<FunctionAnalysisEntry>;
using GroupedFunctionAnalysis = std::vector<FunctionAnalysis>;

inline auto AnalyseFunctions(const std::vector<std::string> &files,
                             const analyzer::metric::MetricExtractor &metric_extractor) {
    FunctionAnalysis analysis;
    analyzer::function::FunctionExtractor extractor;

    for (const auto &filename : files) {
        analyzer::file::File file(filename);
        auto functions = extractor.Get(file);
        for (auto &func : functions) {
            auto metrics = metric_extractor.Get(func);
            analysis.emplace_back(std::move(func), std::move(metrics));
        }
    }

    return analysis;
}

auto SplitByClasses(const auto &analysis) {
    GroupedFunctionAnalysis grouped;
    std::unordered_map<std::string, std::size_t> index;

    for (const auto &entry : analysis) {
        const auto &func = entry.first;
        if (!func.class_name)
            continue;

        std::string key = func.filename;
        key.push_back('\n');
        key.append(*func.class_name);

        auto [it, inserted] = index.emplace(key, grouped.size());
        if (inserted)
            grouped.emplace_back();

        grouped[it->second].push_back(entry);
    }

    return grouped;
}

auto SplitByFiles(const auto &analysis) {
    GroupedFunctionAnalysis grouped;
    std::unordered_map<std::string, std::size_t> index;

    for (const auto &entry : analysis) {
        const auto &filename = entry.first.filename;
        auto [it, inserted] = index.emplace(filename, grouped.size());
        if (inserted)
            grouped.emplace_back();
        grouped[it->second].push_back(entry);
    }

    return grouped;
}

void AccumulateFunctionAnalysis(const auto &analysis,
                                const analyzer::metric_accumulator::MetricsAccumulator &accumulator) {
    for (const auto &entry : analysis)
        accumulator.AccumulateNextFunctionResults(entry.second);
}

}  // namespace analyzer
