#include "metric_impl/cyclomatic_complexity.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "metric_impl/python_ast.hpp"

namespace analyzer::metric::metric_impl {

namespace {

namespace pa = analyzer::metric::python_ast;

bool ShouldSkipBranching(const pa::Node &node, const pa::Node *root) {
    if (&node == root)
        return false;
    return node.type == "function_definition" || node.type == "lambda" || node.type == "class_definition";
}

MetricResult::ValueType CalculateCyclomatic(const pa::Node &body) {
    static const std::unordered_set<std::string_view> simple_nodes = {
        "if_statement",
        "while_statement",
        "for_statement",
        "try_statement",
        "match_statement",
        "assert_statement",
        "conditional_expression"};

    static const std::unordered_set<std::string_view> clause_nodes = {
        "elif_clause",
        "else_clause",
        "case_clause",
        "except_clause",
        "except_group_clause",
        "finally_clause"};

    MetricResult::ValueType complexity = 1;
    std::vector<const pa::Node *> stack;
    stack.push_back(&body);

    while (!stack.empty()) {
        const pa::Node *node = stack.back();
        stack.pop_back();

        const std::string &type = node->type;
        if (simple_nodes.contains(type))
            ++complexity;
        if (clause_nodes.contains(type))
            ++complexity;

        if (ShouldSkipBranching(*node, &body))
            continue;

        stack.reserve(stack.size() + node->children.size());
        for (auto it = node->children.rbegin(); it != node->children.rend(); ++it)
            stack.push_back(&*it);
    }

    return complexity;
}

}  // namespace

MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &f) const {
    if (f.ast.empty())
        return 0;

    pa::Node root = pa::Parse(f.ast);
    if (root.type.empty())
        return 0;

    const pa::Node *body = pa::FindChild(root, "block");
    if (body == nullptr)
        body = &root;

    return CalculateCyclomatic(*body);
}

std::string CyclomaticComplexityMetric::Name() const { return "cyclomatic_complexity"; }

}  // namespace analyzer::metric::metric_impl
