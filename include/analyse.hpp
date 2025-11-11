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
#include <numeric>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
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

namespace detail {

template <typename Range, typename KeySelector>
auto GroupByRange(Range &&range, KeySelector &&key_selector) {
    using State = std::pair<GroupedFunctionAnalysis, std::unordered_map<std::string, std::size_t>>;

    auto view = rv::all(std::forward<Range>(range));
    auto selector = std::forward<KeySelector>(key_selector);

    auto keyed_view = view | rv::transform([&](const auto &entry) {
        return std::pair{selector(entry), std::cref(entry)};
    });

    auto state = rs::fold_left(keyed_view, State{}, [](State state, auto keyed_entry) {
        auto &[grouped, index] = state;
        auto &[key, entry_ref] = keyed_entry;
        auto [it, inserted] = index.try_emplace(std::move(key), grouped.size());
        if (inserted)
            grouped.emplace_back();
        grouped[it->second].push_back(entry_ref.get());
        return state;
    });

    return std::move(state.first);
}

}  // namespace detail

inline auto AnalyseFunctions(const std::vector<std::string> &files,
                             const analyzer::metric::MetricExtractor &metric_extractor) {
    analyzer::function::FunctionExtractor extractor;

    return files
           | rv::transform([&](const std::string &filename) {
                 analyzer::file::File file(filename);
                 return extractor.Get(file);
             })
           | rv::join
           | rv::transform([&](function::Function func) {
                 auto metrics = metric_extractor.Get(func);
                 return FunctionAnalysisEntry{std::move(func), std::move(metrics)};
             })
           | rs::to<FunctionAnalysis>();
}

auto SplitByClasses(const auto &analysis) {
    return detail::GroupByRange(
        analysis | rv::filter([](const auto &entry) { return static_cast<bool>(entry.first.class_name); }),
        [](const auto &entry) {
            auto key = entry.first.filename;
            key.push_back('\n');
            key.append(*entry.first.class_name);
            return key;
        });
}

auto SplitByFiles(const auto &analysis) {
    return detail::GroupByRange(analysis, [](const auto &entry) { return entry.first.filename; });
}

void AccumulateFunctionAnalysis(const auto &analysis,
                                const analyzer::metric_accumulator::MetricsAccumulator &accumulator) {
    rs::fold_left(analysis | rv::elements<1>, std::monostate{},
                  [&accumulator](std::monostate state, const auto &metrics) {
                      accumulator.AccumulateNextFunctionResults(metrics);
                      return state;
                  });
}

}  // namespace analyzer
