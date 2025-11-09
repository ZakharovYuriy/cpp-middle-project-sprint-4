#include "metric_impl/code_lines_count.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "metric_impl/python_ast.hpp"

using namespace std;

namespace analyzer::metric::metric_impl {

std::string readFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filePath);

    std::string data((std::istreambuf_iterator<char>(file)), {});
    if (file.bad())
        throw std::runtime_error("I/O error while reading file: " + filePath);

    if (data.empty())
        throw std::runtime_error("File is empty: " + filePath);

    return data;
}

std::pair<int, int> extractLinesRange(std::string_view str) {
    auto next_num = [&](size_t pos) -> int {
        pos = str.find_first_of("-0123456789", pos);
        if (pos == std::string_view::npos)
            throw std::runtime_error("number expected");

        int value{};
        auto [ptr, ec] = std::from_chars(str.data() + pos, str.data() + str.size(), value);
        if (ec != std::errc{})
            throw std::runtime_error("invalid number");
        return value;
    };

    size_t p = str.find('[');
    if (p == std::string_view::npos)
        throw std::runtime_error("first [ missing");
    int a = next_num(++p);

    p = str.find('[', p);
    if (p == std::string_view::npos)
        throw std::runtime_error("second [ missing");
    int b = next_num(++p);

    return {a, b};
}

static bool checkIsLineComment(const std::vector<std::pair<int, int>> &commentLines, int lineNumber) {
    auto it = std::upper_bound(commentLines.begin(), commentLines.end(), lineNumber,
                               [](int value, const auto &rg) { return value < rg.first; });
    if (it == commentLines.begin())
        return false;
    --it;
    return (lineNumber >= it->first) && (lineNumber <= it->second);
}

MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    constexpr auto delim{"\n"sv};
    constexpr auto commentStr{"comment"sv};

    // First line should contain node with function name and size
    auto functionSizeView = views::split(f.ast, delim) | views::transform([](auto &&comentLine) {
                                return extractLinesRange(string_view(comentLine));
                            });
    if (functionSizeView.begin() == functionSizeView.end())
        throw runtime_error("functionSizeView is empty");
    auto functionSize = *functionSizeView.begin();

    // create vector of [comment.begin, comment.end] numbers
    auto commentLines =
        views::split(f.ast, delim) |
        views::filter([&commentStr](auto &&line) { return ranges::contains_subrange(line, commentStr); }) |
        views::transform([](auto &&comentLine) { return extractLinesRange(string_view(comentLine)); }) |
        ranges::to<vector>();

    // filter empty lines and comment lines
    string buffer = readFile(f.filename);
    auto numberOfLines =
        ranges::distance(views::split(buffer, delim) | views::enumerate | views::filter([&functionSize](auto &&p) {
                             auto [index, line] = p;
                             return index >= functionSize.first && index <= functionSize.second;
                         }) |
                         views::filter([&commentLines](auto &&pair) {
                             const auto &[index, line] = pair;
                             bool isEmptyLine = ranges::all_of(line, [](unsigned char ch) { return isspace(ch); });
                             bool isComment = checkIsLineComment(commentLines, index);
                             return !isEmptyLine && !isComment;
                         }));

    return numberOfLines;
}

std::string CodeLinesCountMetric::Name() const { return "code_lines_count"; }

}  // namespace analyzer::metric::metric_impl
