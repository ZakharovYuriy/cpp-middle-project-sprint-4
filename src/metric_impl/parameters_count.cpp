#include "metric_impl/parameters_count.hpp"

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
#include <string_view>
#include <variant>
#include <vector>

#include "metric_impl/python_ast.hpp"

namespace analyzer::metric::metric_impl {
namespace {

namespace pa = analyzer::metric::python_ast;

bool IsParameterNodeType(std::string_view type) {
    if (type == "identifier")
        return true;
    if (type == "tuple_pattern" || type == "list_pattern" || type == "parenthesized_pattern")
        return true;
    if (type == "list_splat_pattern" || type == "dictionary_splat_pattern")
        return true;
    if (type.find("parameter") != std::string_view::npos)
        return type != "positional_separator" && type != "keyword_separator";
    return false;
}

MetricResult::ValueType CountParameters(const pa::Node &parameters) {
    MetricResult::ValueType count = 0;
    for (const auto &child : parameters.children)
        if (IsParameterNodeType(child.type))
            ++count;
    return count;
}

}  // namespace

MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &f) const {
    if (f.ast.empty())
        return 0;

    pa::Node root = pa::Parse(f.ast);
    if (root.type.empty())
        return 0;

    const pa::Node *parameters = pa::FindChild(root, "parameters");
    if (parameters == nullptr)
        return 0;

    return CountParameters(*parameters);
}

std::string CountParametersMetric::Name() const { return "parameters_count"; }

}  // namespace analyzer::metric::metric_impl
