#pragma once
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "metric_accumulator.hpp"

namespace analyzer::metric_accumulator::metric_accumulator_impl {

struct AverageAccumulator : public IAccumulator {
    void Accumulate(const metric::MetricResult &metric_result) override;

    void Finalize() override;

    void Reset();

    double Get() const;

private:
    int sum = 0;
    int count = 0;
    double average = 0;
};

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
