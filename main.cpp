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

void RunDebugSnippet() {
    try {
        analyzer::file::File file("files/sample.py");
        std::cout << file.ast;

        analyzer::function::FunctionExtractor extractor;
        const auto functions = extractor.Get(file);
        for (const auto &func : functions) {
            std::cout << "\n--- Function: " << func.name << " ---\n" << func.ast;
        }
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

    for (const auto &metric : entry.second)
        std::cout << indent << "  " << metric.metric_name << ": " << metric.value << '\n';
}

void PrintAnalysisSummary(const analyzer::FunctionAnalysis &analysis) {
    if (analysis.empty()) {
        std::cout << "Функции не найдены.\n";
        return;
    }

    std::cout << "Метрики по функциям:\n";
    for (const auto &entry : analysis)
        PrintFunctionMetrics(entry, "  ", true);
}

void PrintGroupedAnalysis(std::string_view title, const analyzer::GroupedFunctionAnalysis &grouped,
                          const std::function<std::string(const analyzer::function::Function &)> &header_formatter) {
    if (grouped.empty())
        return;

    std::cout << '\n' << title << ":\n";
    for (const auto &group : grouped) {
        if (group.empty())
            continue;

        std::cout << "  " << header_formatter(group.front().first) << '\n';
        for (const auto &entry : group)
            PrintFunctionMetrics(entry, "    ", false);
    }
}

bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size())
        return false;
    for (size_t i = 0; i < lhs.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(lhs[i])) != std::tolower(static_cast<unsigned char>(rhs[i])))
            return false;
    return true;
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

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
