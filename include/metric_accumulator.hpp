#pragma once
#include <unistd.h>

#include <algorithm>
#include <any>
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
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "metric.hpp"

namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyzer::metric_accumulator {

struct IAccumulator {
    virtual void Accumulate(const metric::MetricResult &metric_result) = 0;
    virtual void Finalize() = 0;
    virtual void Reset() = 0;
    virtual ~IAccumulator() = default;

protected:
    bool is_finalized = false;
};

struct MetricsAccumulator {
    template <typename Accumulator>
    void RegisterAccumulator(const std::string &metric_name, std::unique_ptr<Accumulator> acc) {
        static_assert(std::is_base_of_v<IAccumulator, Accumulator>,
                      "Accumulator must derive from IAccumulator");
        if (!acc)
            throw std::invalid_argument("Accumulator pointer is null");
        accumulators[metric_name] = std::shared_ptr<IAccumulator>(std::move(acc));
    }
    template <typename Accumulator>
    const Accumulator &GetFinalizedAccumulator(const std::string &metric_name) const {
        static_assert(std::is_base_of_v<IAccumulator, Accumulator>,
                      "Accumulator must derive from IAccumulator");
        auto it = accumulators.find(metric_name);
        if (it == accumulators.end())
            throw std::out_of_range("Accumulator for metric '" + metric_name + "' not found");
        auto typed = std::dynamic_pointer_cast<Accumulator>(it->second);
        if (!typed)
            throw std::runtime_error("Accumulator type mismatch for metric '" + metric_name + "'");
        typed->Finalize();
        return *typed;
    }
    void AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const;

    void ResetAccumulators();

private:
    std::unordered_map<std::string, std::shared_ptr<IAccumulator>> accumulators;
};

}  // namespace analyzer::metric_accumulator
