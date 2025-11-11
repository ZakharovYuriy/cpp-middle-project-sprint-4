#include "metric_impl/parameters_count.hpp"

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
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

using namespace std;

namespace analyzer::metric::metric_impl {

static const std::unordered_set<std::string_view> parameterNodeTypes = {
    "identifier",    "typed_parameter",    "default_parameter",        "typed_default_parameter",
    "tuple_pattern", "list_splat_pattern", "dictionary_splat_pattern",
};

int findFirstOpenParen(auto &&ast, std::size_t startSearchFrom) {
    if (startSearchFrom >= ranges::distance(ast))
        throw std::out_of_range("startSearchFrom is past the end");

    auto v = ast | std::views::drop(startSearchFrom) | std::views::enumerate;

    auto it = std::ranges::find_if(v, [](const auto &pos_ch) {
        const auto &[pos, ch] = pos_ch;
        (void)pos;
        return ch == '(';
    });

    if (it == std::ranges::end(v))
        throw std::runtime_error("No opening parenthesis found");

    const auto &[relPos, ch] = *it;
    return static_cast<int>(relPos + startSearchFrom);
}

optional<int> findPositionOfClosingParenthesis(auto &&ast, std::size_t startSearchFrom = 0) {
    int buff = 0;
    int positionOfFistOpeningParenthesis = findFirstOpenParen(ast, startSearchFrom);
    auto bracketDeltas = ast | views::enumerate | views::drop(positionOfFistOpeningParenthesis) |
                         std::views::transform([](auto &&pos_ch) {
                             const auto &[pos, ch] = pos_ch;
                             return std::pair{pos, (ch == '(') - (ch == ')')};
                         });
    auto it = std::ranges::find_if(bracketDeltas, [&buff](const auto &pos_delta) {
        const auto &[pos, delta] = pos_delta;
        buff += delta;
        return buff == 0;
    });
    if (it == std::ranges::end(bracketDeltas))
        return nullopt;
    auto [position, delta] = *it;
    return position;
}

MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &f) const {
    constexpr auto delim{"\n"sv};
    constexpr auto parametersStr{"parameters"sv};

    auto parametersLine =
        views::split(f.ast, delim) | views::enumerate | views::filter([&parametersStr](auto &&positionAndLine) {
            const auto &[position, line] = positionAndLine;
            return ranges::contains_subrange(line, parametersStr);
        });

    if (parametersLine.begin() == parametersLine.end())
        throw runtime_error("parametersLines is empty");
    const auto FunctionParametersLine = parametersLine.front();
    const auto &[startOfParmsLine, line] = FunctionParametersLine;
    const auto symbolsBeforLineWithParams = ranges::fold_left(
        views::split(f.ast, delim) | views::take(startOfParmsLine) | views::transform([](auto &&line) {
            return line.size() + 1;  // count removed '\n'
        }),
        0, std::plus{});

    const auto endOfParmsPosition = findPositionOfClosingParenthesis(f.ast, symbolsBeforLineWithParams);

    if (!endOfParmsPosition)
        throw std::runtime_error("No closing parenthesis found");

    auto paramsLines = f.ast | views::take(*endOfParmsPosition) | views::drop(symbolsBeforLineWithParams) |
                       views::split(delim) | views::drop(1);

    auto groupedParams = (ranges::fold_left(paramsLines, pair{vector<string>{}, vector<vector<string>>{}},
                                            [](auto acc, const auto &line) {
                                                auto &[accum, out] = acc;
                                                accum.push_back(string(line.begin(), line.end()));
                                                if (findPositionOfClosingParenthesis(accum | views::join, 0)) {
                                                    out.push_back(std::move(accum));
                                                    accum.clear();
                                                }
                                                return acc;
                                            }))
                             .second;

    auto mainNodesOfParams = groupedParams | views::transform([](const auto &group) -> string_view {
                                 return group.empty() ? string_view{} : string_view(group.front());
                             });

    return ranges::distance(mainNodesOfParams | views::filter([](auto &&line) {
                                return ranges::any_of(parameterNodeTypes, [&line](auto &&node) {
                                    return ranges::contains_subrange(line, node);
                                });
                            }));
}

std::string CountParametersMetric::Name() const { return "parameters_count"; }

}  // namespace analyzer::metric::metric_impl
