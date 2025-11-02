#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string_view>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::metric_impl {
namespace {

std::filesystem::path TestFilePath() {
    return std::filesystem::path(__FILE__).parent_path() / "files" / "code_lines_count_sample.py";
}

std::filesystem::path SparseSampleFilePath() {
    return std::filesystem::path(__FILE__).parent_path() / "files" / "code_lines_count_sparse.py";
}

function::Function GetFunction(std::string_view function_name,
                               std::filesystem::path sample_path = TestFilePath()) {
    analyzer::file::File file(sample_path.string());
    function::FunctionExtractor extractor;
    auto functions = extractor.Get(file);

    auto it = std::find_if(functions.begin(), functions.end(), [&](const function::Function &func) {
        return func.name == function_name;
    });

    if (it == functions.end()) {
        throw std::runtime_error("Function " + std::string(function_name) +
                                 " not found in " + sample_path.string());
    }

    return *it;
}

}  // namespace

TEST(CodeLinesCountMetric, CountsClassMethodWithoutDocstring) {
    const auto function = GetFunction("__init__");
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(result.value, 2);
}

TEST(CodeLinesCountMetric, SkipsDocstringAndComments) {
    const auto function = GetFunction("helper_function");
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(result.value, 4);
}

TEST(CodeLinesCountMetric, IgnoresBlankLinesInSparseFunction) {
    const auto function = GetFunction("example_function", SparseSampleFilePath());
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(result.value, 13);
}

}  // namespace analyzer::metric::metric_impl
