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

}  // namespace

int main(int argc, char *argv[]) {
    analyzer::cmd::ProgramOptions options;
    // распарсите входные параметры

    RunDebugSnippet();

    // analyzer::metric::MetricExtractor metric_extractor;
    // зарегистрируйте метрики в metric_extractor

    // запустите analyzer::AnalyseFunctions
    // выведете результаты анализа на консоль

    // analyzer::metric_accumulator::MetricsAccumulator accumulator;
    // зарегистрируйте аккумуляторы метрик в accumulator

    // запустите analyzer::SplitByFiles
    // запустите analyzer::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
    // выведете результаты на консоль

    // запустите analyzer::SplitByClasses
    // запустите analyzer::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
    // выведете результаты на консоль

    // запустите analyzer::AccumulateFunctionAnalysis для всех результатов метрик
    // выведете результаты на консоль

    return 0;
}
