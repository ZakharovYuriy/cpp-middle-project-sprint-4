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

struct CategoricalAccumulator : public IAccumulator {
    void Accumulate(const metric::MetricResult &metric_result) override;

    virtual void Finalize() override;

    virtual void Reset() override;

    const std::unordered_map<std::string, int> &Get() const;

private:
    std::unordered_map<std::string, int> categories_freq;
};

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
