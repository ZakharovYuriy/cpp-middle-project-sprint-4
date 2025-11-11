#include "analyse.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <variant>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyzer::tests {

namespace {

std::filesystem::path SampleFileOne() {
    return std::filesystem::path(__FILE__).parent_path() / "files" / "analysis_sample_one.py";
}

std::filesystem::path SampleFileTwo() {
    return std::filesystem::path(__FILE__).parent_path() / "files" / "analysis_sample_two.py";
}

struct NameLengthMetric : analyzer::metric::IMetric {
protected:
    analyzer::metric::MetricResult::ValueType CalculateImpl(const analyzer::function::Function &f) const override {
        return static_cast<int>(f.name.size());
    }

    std::string Name() const override { return "name_length"; }
};

struct SumAccumulator : analyzer::metric_accumulator::IAccumulator {
    int total = 0;

    void Accumulate(const analyzer::metric::MetricResult &metric_result) override {
        total += std::get<int>(metric_result.value);
    }

    void Finalize() override { is_finalized = true; }

    void Reset() override {
        total = 0;
        is_finalized = false;
    }
};

std::vector<std::string> SampleFiles() {
    return {SampleFileOne().string(), SampleFileTwo().string()};
}

analyzer::metric::MetricExtractor BuildExtractor() {
    analyzer::metric::MetricExtractor extractor;
    extractor.RegisterMetric(std::make_unique<NameLengthMetric>());
    return extractor;
}

int ExpectedTotalNameLength(const analyzer::FunctionAnalysis &analysis) {
    return std::accumulate(analysis.begin(), analysis.end(), 0,
                           [](int acc, const analyzer::FunctionAnalysisEntry &entry) {
                               return acc + static_cast<int>(entry.first.name.size());
                           });
}

}  // namespace

TEST(AnalyseFunctions, CollectsFunctionsAndMetrics) {
    auto extractor = BuildExtractor();
    const auto analysis = AnalyseFunctions(SampleFiles(), extractor);

    EXPECT_EQ(analysis.size(), 5);
    for (const auto &entry : analysis) {
        const auto &function = entry.first;
        const auto &metrics = entry.second;

        EXPECT_EQ(metrics.size(), 1u);
        EXPECT_EQ(metrics.front().metric_name, "name_length");
        ASSERT_TRUE(std::holds_alternative<int>(metrics.front().value));
        EXPECT_EQ(std::get<int>(metrics.front().value), static_cast<int>(function.name.size()));
        EXPECT_FALSE(function.name.empty());
    }
}

TEST(AnalyseFunctions, SplitByClassesGroupsClassMethods) {
    auto extractor = BuildExtractor();
    const auto analysis = AnalyseFunctions(SampleFiles(), extractor);

    const auto grouped = SplitByClasses(analysis);
    ASSERT_EQ(grouped.size(), 2u);

    for (const auto &group : grouped) {
        ASSERT_FALSE(group.empty());
        const auto &first_function = group.front().first;
        ASSERT_TRUE(first_function.class_name.has_value());
        for (const auto &entry : group) {
            EXPECT_TRUE(entry.first.class_name.has_value());
            EXPECT_EQ(entry.first.filename, first_function.filename);
            EXPECT_EQ(entry.first.class_name, first_function.class_name);
        }
    }
}

TEST(AnalyseFunctions, SplitByFilesGroupsFunctionsByFilename) {
    auto extractor = BuildExtractor();
    const auto analysis = AnalyseFunctions(SampleFiles(), extractor);

    const auto grouped = SplitByFiles(analysis);
    ASSERT_EQ(grouped.size(), 2u);

    EXPECT_TRUE(std::all_of(grouped[0].begin(), grouped[0].end(), [&](const auto &entry) {
        return entry.first.filename == SampleFileOne().string();
    }));
    EXPECT_TRUE(std::all_of(grouped[1].begin(), grouped[1].end(), [&](const auto &entry) {
        return entry.first.filename == SampleFileTwo().string();
    }));
}

TEST(AnalyseFunctions, AccumulateFunctionAnalysisFeedsAccumulator) {
    auto extractor = BuildExtractor();
    const auto analysis = AnalyseFunctions(SampleFiles(), extractor);

    analyzer::metric_accumulator::MetricsAccumulator accumulator;
    accumulator.RegisterAccumulator("name_length", std::make_unique<SumAccumulator>());

    AccumulateFunctionAnalysis(analysis, accumulator);

    const auto &sum_acc =
        accumulator.GetFinalizedAccumulator<SumAccumulator>("name_length");
    EXPECT_EQ(sum_acc.total, ExpectedTotalNameLength(analysis));
}

}  // namespace analyzer::tests
