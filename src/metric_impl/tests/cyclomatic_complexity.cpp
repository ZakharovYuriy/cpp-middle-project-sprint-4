#include "metric_impl/cyclomatic_complexity.hpp"

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
    return GetFunctionFromFile(function_name, SamplePath("metrics_sample.py"));
}

struct CyclomaticSampleCase {
    std::string filename;
    std::string function_name;
    int expected_complexity;
};

class CyclomaticComplexityMetricSamples : public ::testing::TestWithParam<CyclomaticSampleCase> {};

}  // namespace

TEST(CyclomaticComplexityMetric, HandlesRichControlFlow) {
    const auto function = GetFunction("complexity_target");
    CyclomaticComplexityMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 15);
}

TEST_P(CyclomaticComplexityMetricSamples, MatchesExpectedComplexityAcrossSamples) {
    const auto params = GetParam();
    const auto function = GetFunctionFromFile(params.function_name, SamplePath(params.filename));

    CyclomaticComplexityMetric metric;
    const auto result = metric.Calculate(function);

    ASSERT_TRUE(std::holds_alternative<int>(result.value));
    EXPECT_EQ(std::get<int>(result.value), params.expected_complexity);
}

const std::vector<CyclomaticSampleCase> &CyclomaticTestCases() {
    static const std::vector<CyclomaticSampleCase> cases = {
        {"code_lines_count_sample.py", "__init__", 0},
        {"code_lines_count_sample.py", "process", 0},
        {"code_lines_count_sample.py", "helper_function", 1},
        {"code_lines_count_sparse.py", "example_function", 2},
        {"code_lines_count_sparse.py", "__init__", 0},
        {"code_lines_count_sparse.py", "do_something", 0},
        {"code_lines_count_sparse.py", "another_func", 0},
        {"code_lines_count_sparse.py", "decorated_function", 0},
        {"comments.py", "Func_comments", 0},
        {"exceptions.py", "Try_Exceptions", 4},
        {"if.py", "testIf", 1},
        {"loops.py", "TestLoops", 3},
        {"many_lines.py", "testmultiline", 1},
        {"many_parameters.py", "__test_multiparameters__", 1},
        {"match_case.py", "test_Match_case", 4},
        {"metrics_sample.py", "complexity_target", 15},
        {"metrics_sample.py", "params_target", 0},
        {"nested_if.py", "Testnestedif", 5},
        {"simple.py", "test_simple", 1},
        {"ternary.py", "teSt_ternary", 2},
    };
    return cases;
}

INSTANTIATE_TEST_SUITE_P(
    AllMetricSamples, CyclomaticComplexityMetricSamples, ::testing::ValuesIn(CyclomaticTestCases()),
    [](const ::testing::TestParamInfo<CyclomaticSampleCase> &info) {
        return analyzer::metric::tests::ComposeParamName(info.param.filename, info.param.function_name);
    });

}  // namespace analyzer::metric::metric_impl
