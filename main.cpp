#include <unistd.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "analyse.hpp"
#include "cmd_options.hpp"
#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"
#include "metric_accumulator_impl/accumulators.hpp"
#include "metric_impl/metrics.hpp"

namespace {
namespace rv = std::ranges::views;
namespace rs = std::ranges;

using SumAverageAccumulator = analyzer::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator;
using SumAverageStats = SumAverageAccumulator::SumAverage;

struct AggregatedMetric {
    std::string metric_name;
    SumAverageStats stats;
};

constexpr std::array<std::string_view, 3> kAggregatedMetricNames = {
    "code_lines_count", "cyclomatic_complexity", "parameters_count"};

std::string FormatMetricValue(const analyzer::metric::MetricResult &metric) {
    return std::visit(
        [](const auto &value) -> std::string {
            using ValueType = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<ValueType, std::string>)
                return value;
            else
                return std::to_string(value);
        },
        metric.value);
}

std::string FormatAverage(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

analyzer::metric_accumulator::MetricsAccumulator BuildSumAverageAccumulator() {
    analyzer::metric_accumulator::MetricsAccumulator accumulator;
    rs::for_each(kAggregatedMetricNames, [&](std::string_view metric_name) {
        accumulator.RegisterAccumulator(std::string(metric_name), std::make_unique<SumAverageAccumulator>());
    });
    return accumulator;
}

std::vector<AggregatedMetric> AggregateMetrics(const analyzer::FunctionAnalysis &analysis) {
    auto accumulator = BuildSumAverageAccumulator();
    analyzer::AccumulateFunctionAnalysis(analysis, accumulator);
    auto aggregated =
        kAggregatedMetricNames
        | rv::transform([&](std::string_view metric_name) {
              const auto &acc =
                  accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(std::string(metric_name));
              return AggregatedMetric{std::string(metric_name), acc.Get()};
          })
        | rs::to<std::vector>();
    accumulator.ResetAccumulators();
    return aggregated;
}

void PrintAggregatedMetrics(std::string_view indent, const std::vector<AggregatedMetric> &metrics) {
    rs::for_each(metrics, [&](const AggregatedMetric &metric) {
        std::cout << indent << metric.metric_name << ": sum=" << metric.stats.sum
                  << ", avg=" << FormatAverage(metric.stats.average) << '\n';
    });
}

void RunDebugSnippet() {
    try {
        analyzer::file::File file("files/sample.py");
        std::cout << file.ast;

        analyzer::function::FunctionExtractor extractor;
        const auto functions = extractor.Get(file);
        rs::for_each(functions, [](const analyzer::function::Function &func) {
            std::cout << "\n--- Function: " << func.name << " ---\n" << func.ast;
        });
        std::cout << '\n';
    } catch (const std::exception &e) {
        std::cerr << "Debug snippet failed: " << e.what() << '\n';
    }
}

void PrintFunctionMetrics(const analyzer::FunctionAnalysisEntry &entry, const std::string &indent,
                          bool include_filename) {
    const auto &func = entry.first;
    std::cout << indent;
    if (include_filename)
        std::cout << func.filename << " :: ";
    if (func.class_name)
        std::cout << *func.class_name << '.';
    std::cout << func.name << '\n';

    rs::for_each(entry.second, [&](const analyzer::metric::MetricResult &metric) {
        std::cout << indent << "  " << metric.metric_name << ": " << FormatMetricValue(metric) << '\n';
    });
}

void PrintAnalysisSummary(const analyzer::FunctionAnalysis &analysis) {
    if (analysis.empty()) {
        std::cout << "Функции не найдены.\n";
        return;
    }

    std::cout << "Метрики по функциям:\n";
    rs::for_each(analysis, [](const analyzer::FunctionAnalysisEntry &entry) {
        PrintFunctionMetrics(entry, "  ", true);
    });
}

void PrintGroupedAnalysis(std::string_view title, const analyzer::GroupedFunctionAnalysis &grouped,
                          const std::function<std::string(const analyzer::function::Function &)> &header_formatter) {
    if (grouped.empty())
        return;

    std::cout << '\n' << title << ":\n";
    rs::for_each(grouped, [&](const analyzer::FunctionAnalysis &group) {
        if (group.empty())
            return;

        std::cout << "  " << header_formatter(group.front().first) << '\n';
        rs::for_each(group, [](const analyzer::FunctionAnalysisEntry &entry) {
            PrintFunctionMetrics(entry, "    ", false);
        });
    });
}

void PrintAggregatedSummary(std::string_view title, const analyzer::FunctionAnalysis &analysis) {
    if (analysis.empty())
        return;

    std::cout << '\n' << title << ":\n";
    PrintAggregatedMetrics("  ", AggregateMetrics(analysis));
}

template <typename HeaderFormatter>
void PrintGroupedAggregations(std::string_view title, const analyzer::GroupedFunctionAnalysis &grouped,
                              HeaderFormatter &&header_formatter) {
    if (grouped.empty())
        return;

    std::cout << '\n' << title << ":\n";
    rs::for_each(grouped, [&](const analyzer::FunctionAnalysis &group) {
        if (group.empty())
            return;

        std::cout << "  " << header_formatter(group.front().first) << '\n';
        PrintAggregatedMetrics("    ", AggregateMetrics(group));
    });
}

bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs) {
    return lhs.size() == rhs.size() &&
           std::ranges::equal(lhs, rhs, [](char l, char r) {
               return std::tolower(static_cast<unsigned char>(l)) == std::tolower(static_cast<unsigned char>(r));
           });
}
}  // namespace

int main(int argc, char *argv[]) {
    analyzer::cmd::ProgramOptions options;

    if (!options.Parse(argc, argv))
        return options.IsHelpRequested() ? EXIT_SUCCESS : EXIT_FAILURE;

    if (options.DebugEnabled())
        RunDebugSnippet();

    analyzer::metric::MetricExtractor metric_extractor;
    metric_extractor.RegisterMetric(std::make_unique<analyzer::metric::metric_impl::CodeLinesCountMetric>());
    metric_extractor.RegisterMetric(std::make_unique<analyzer::metric::metric_impl::CyclomaticComplexityMetric>());
    metric_extractor.RegisterMetric(std::make_unique<analyzer::metric::metric_impl::CountParametersMetric>());

    try {
        const auto &files = options.GetFiles();
        auto analysis = analyzer::AnalyseFunctions(files, metric_extractor);

        PrintAnalysisSummary(analysis);

        auto grouped_by_file = analyzer::SplitByFiles(analysis);
        PrintGroupedAnalysis("Метрики по файлам", grouped_by_file,
                             [](const analyzer::function::Function &func) { return "Файл: " + func.filename; });

        auto grouped_by_class = analyzer::SplitByClasses(analysis);
        PrintGroupedAnalysis("Метрики по классам", grouped_by_class, [](const analyzer::function::Function &func) {
            std::string header = "Класс: ";
            if (func.class_name)
                header += *func.class_name;
            else
                header += "<без имени>";
            header += " (файл " + func.filename + ')';
            return header;
        });

        PrintAggregatedSummary("Сводные метрики по всем функциям", analysis);
        PrintGroupedAggregations("Сводные метрики по файлам", grouped_by_file,
                                 [](const analyzer::function::Function &func) { return "Файл: " + func.filename; });
        PrintGroupedAggregations(
            "Сводные метрики по классам", grouped_by_class, [](const analyzer::function::Function &func) {
                std::string header = "Класс: ";
                if (func.class_name)
                    header += *func.class_name;
                else
                    header += "<без имени>";
                header += " (файл " + func.filename + ')';
                return header;
            });

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
