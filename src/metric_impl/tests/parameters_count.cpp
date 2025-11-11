#include "metric_impl/parameters_count.hpp"

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

struct ParametersSampleCase {
    std::string filename;
    std::string function_name;
    int expected_parameters;
};

class ParametersCountMetricSamples : public ::testing::TestWithParam<ParametersSampleCase> {};

}  // namespace

TEST(CountParametersMetric, CountsDifferentKinds) {
    const auto function = GetFunction("params_target");
    CountParametersMetric metric;

    const auto result = metric.Calculate(function);

    EXPECT_EQ(std::get<int>(result.value), 8);
}

TEST_P(ParametersCountMetricSamples, CountsParametersAcrossSamples) {
    const auto params = GetParam();
    const auto function = GetFunctionFromFile(params.function_name, SamplePath(params.filename));

    CountParametersMetric metric;
    const auto result = metric.Calculate(function);

    ASSERT_TRUE(std::holds_alternative<int>(result.value));
    EXPECT_EQ(std::get<int>(result.value), params.expected_parameters);
}

const std::vector<ParametersSampleCase> &ParametersTestCases() {
    static const std::vector<ParametersSampleCase> cases = {
        {"code_lines_count_sample.py", "__init__", 3},
        {"code_lines_count_sample.py", "process", 2},
        {"code_lines_count_sample.py", "helper_function", 0},
        {"code_lines_count_sparse.py", "example_function", 2},
        {"code_lines_count_sparse.py", "__init__", 2},
        {"code_lines_count_sparse.py", "do_something", 1},
        {"code_lines_count_sparse.py", "another_func", 0},
        {"code_lines_count_sparse.py", "decorated_function", 0},
        {"comments.py", "Func_comments", 3},
        {"exceptions.py", "Try_Exceptions", 0},
        {"if.py", "testIf", 1},
        {"loops.py", "TestLoops", 1},
        {"many_lines.py", "testmultiline", 0},
        {"many_parameters.py", "__test_multiparameters__", 5},
        {"match_case.py", "test_Match_case", 1},
        {"metrics_sample.py", "complexity_target", 3},
        {"metrics_sample.py", "params_target", 8},
        {"nested_if.py", "Testnestedif", 2},
        {"simple.py", "test_simple", 0},
        {"ternary.py", "teSt_ternary", 1},
    };
    return cases;
}

INSTANTIATE_TEST_SUITE_P(
    AllMetricSamples, ParametersCountMetricSamples, ::testing::ValuesIn(ParametersTestCases()),
    [](const ::testing::TestParamInfo<ParametersSampleCase> &info) {
        return analyzer::metric::tests::ComposeParamName(info.param.filename, info.param.function_name);
    });

}  // namespace analyzer::metric::metric_impl
