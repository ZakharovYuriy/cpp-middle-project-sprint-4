#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <variant>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::metric_impl {
namespace {

std::filesystem::path MetricsSamplePath() {
    return std::filesystem::path(__FILE__).parent_path() / "files" / "metrics_sample.py";
}

function::Function GetFunction(std::string_view function_name) {
    analyzer::file::File file(MetricsSamplePath().string());
    function::FunctionExtractor extractor;
    auto functions = extractor.Get(file);

    auto it = std::find_if(functions.begin(), functions.end(), [&](const function::Function &func) {
        return func.name == function_name;
    });

    if (it == functions.end())
        throw std::runtime_error("Function not found");

    return *it;
}

}  // namespace

TEST(CountParametersMetric, CountsDifferentKinds) {
    const auto function = GetFunction("params_target");
    CountParametersMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 8);
}

}  // namespace analyzer::metric::metric_impl
