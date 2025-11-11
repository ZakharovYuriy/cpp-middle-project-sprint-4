#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "test_utils.hpp"

namespace analyzer::metric::metric_impl {
namespace {

const std::filesystem::path &SamplesDir() {
    static const std::filesystem::path dir = analyzer::metric::tests::SamplesDir(__FILE__);
    return dir;
}

std::filesystem::path SamplePath(std::string_view filename) {
    return SamplesDir() / std::filesystem::path(std::string(filename));
}

function::Function GetFunctionFromFile(std::string_view function_name, const std::filesystem::path &sample_path) {
    return analyzer::metric::tests::LoadFunction(function_name, sample_path);
}

function::Function GetFunction(std::string_view function_name) {
    return GetFunctionFromFile(function_name, SamplePath("code_lines_count_sample.py"));
}

struct CodeLinesSampleCase {
    std::string filename;
    std::string function_name;
    int expected_lines;
};

class CodeLinesCountMetricSamples : public ::testing::TestWithParam<CodeLinesSampleCase> {};

}  // namespace

TEST(CodeLinesCountMetric, CountsClassMethodWithoutDocstring) {
    const auto function = GetFunction("__init__");
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 3);
}

TEST(CodeLinesCountMetric, CountsDocstringsButSkipsComments) {
    const auto function = GetFunction("helper_function");
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 6);
}

TEST(CodeLinesCountMetric, IgnoresBlankLinesInSparseFunction) {
    const auto function = GetFunctionFromFile("example_function", SamplePath("code_lines_count_sparse.py"));
    CodeLinesCountMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 18);
}

TEST_P(CodeLinesCountMetricSamples, MatchesExpectedValueAcrossSamples) {
    const auto params = GetParam();
    const auto function = GetFunctionFromFile(params.function_name, SamplePath(params.filename));

    CodeLinesCountMetric metric;
    const auto result = metric.Calculate(function);

    ASSERT_TRUE(std::holds_alternative<int>(result.value));
    EXPECT_EQ(std::get<int>(result.value), params.expected_lines);
}

const std::vector<CodeLinesSampleCase> &CodeLinesTestCases() {
    static const std::vector<CodeLinesSampleCase> cases = {
        {"code_lines_count_sample.py", "__init__", 3},
        {"code_lines_count_sample.py", "process", 4},
        {"code_lines_count_sample.py", "helper_function", 6},
        {"code_lines_count_sparse.py", "example_function", 18},
        {"code_lines_count_sparse.py", "__init__", 5},
        {"code_lines_count_sparse.py", "do_something", 4},
        {"code_lines_count_sparse.py", "another_func", 2},
        {"code_lines_count_sparse.py", "decorated_function", 2},
        {"comments.py", "Func_comments", 4},
        {"exceptions.py", "Try_Exceptions", 8},
        {"if.py", "testIf", 4},
        {"loops.py", "TestLoops", 7},
        {"many_lines.py", "testmultiline", 12},
        {"many_parameters.py", "__test_multiparameters__", 2},
        {"match_case.py", "test_Match_case", 8},
        {"metrics_sample.py", "complexity_target", 29},
        {"metrics_sample.py", "params_target", 2},
        {"nested_if.py", "Testnestedif", 9},
        {"simple.py", "test_simple", 6},
        {"ternary.py", "teSt_ternary", 2},
    };
    return cases;
}

INSTANTIATE_TEST_SUITE_P(
    AllMetricSamples, CodeLinesCountMetricSamples, ::testing::ValuesIn(CodeLinesTestCases()),
    [](const ::testing::TestParamInfo<CodeLinesSampleCase> &info) {
        return analyzer::metric::tests::ComposeParamName(info.param.filename, info.param.function_name);
    });

}  // namespace analyzer::metric::metric_impl
