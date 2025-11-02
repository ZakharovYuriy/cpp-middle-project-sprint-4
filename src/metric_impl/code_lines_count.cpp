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

static vector<string> ReadFileLines(const string &path, bool &ok) {
    ifstream input(path);
    vector<string> lines;
    if (!input.is_open()) {
        ok = false;
        return lines;
    }
    ok = true;
    string line;
    while (getline(input, line))
        lines.push_back(line);
    return lines;
}

static bool HasNonWhitespace(string_view line) {
    for (char ch : line)
        if (!isspace(static_cast<unsigned char>(ch)))
            return true;
    return false;
}

namespace pa = ::analyzer::metric::python_ast;
using Node = pa::Node;

// отрезок в рамках одной строки: [c0, c1)
struct Seg {
    int c0, c1;
};
using LineSegs = unordered_map<int, vector<Seg>>;  // line -> непересекающиеся сегменты

// добавление сегмента с нормализацией (склейка пересечений)
static void add_seg(LineSegs &m, int line, Seg s) {
    if (s.c1 <= s.c0)
        return;
    auto &v = m[line];
    v.push_back(s);
    // склеим
    sort(v.begin(), v.end(), [](auto &a, auto &b) { return a.c0 < b.c0; });
    vector<Seg> merged;
    for (auto &x : v) {
        if (merged.empty() || x.c0 > merged.back().c1)
            merged.push_back(x);
        else
            merged.back().c1 = max(merged.back().c1, x.c1);
    }
    v.swap(merged);
}

// вычесть из большого сегмента lineSeg набор child-сегментов на той же строке
static vector<Seg> subtract_line_seg(Seg big, const vector<Seg> &children) {
    vector<Seg> out;
    int cur = big.c0;
    for (auto &c : children) {
        if (c.c0 > cur)
            out.push_back({cur, min(c.c0, big.c1)});
        cur = max(cur, c.c1);
        if (cur >= big.c1)
            break;
    }
    if (cur < big.c1)
        out.push_back({cur, big.c1});
    // удалить пустые
    out.erase(remove_if(out.begin(), out.end(), [](auto s) { return s.c1 <= s.c0; }), out.end());
    return out;
}

// диапазон узла разбитый на построчные сегменты (без детей)
static vector<pair<int, Seg>> node_span_per_lines(const Node &n) {
    if (n.start.line != n.end.line)
        return {};
    return {{n.start.line, {n.start.col, n.end.col}}};
}

// объединение покрытий (склеивает сегменты)
static void merge_cover(LineSegs &dst, const LineSegs &src) {
    for (auto &[ln, v] : src)
        for (auto s : v)
            add_seg(dst, ln, s);
}

static void collect_special_lines(const Node &n, const string *parent_type,
                                  unordered_set<int> &docstring_lines,
                                  unordered_set<int> &comment_lines,
                                  unordered_set<int> &structural_only_lines) {
    if (n.type == "comment") {
        for (int ln = n.start.line; ln <= n.end.line; ++ln)
            comment_lines.insert(ln);
    }

    const bool is_docstring_expr = n.type == "expression_statement" && !n.children.empty() &&
                                   all_of(n.children.begin(), n.children.end(), [](const Node &child) {
                                       return child.type == "string" || child.type == "comment";
                                   });
    const bool is_block_docstring = n.type == "string" && parent_type && *parent_type == "block";
    if (is_docstring_expr || is_block_docstring) {
        for (int ln = n.start.line; ln <= n.end.line; ++ln)
            docstring_lines.insert(ln);
    }

    if (n.type == "parenthesized_expression" && n.start.line != n.end.line)
        structural_only_lines.insert(n.end.line);

    for (const auto &child : n.children)
        collect_special_lines(child, &n.type, docstring_lines, comment_lines, structural_only_lines);
}

// посчитать «покрытие» узла: возвращает сегменты «значимого» кода (по детям + собственный остаток)
static LineSegs covered_segs(const Node &n, const string *parent_type = nullptr) {
    LineSegs cov_children;
    for (auto &ch : n.children) {
        auto sub = covered_segs(ch, &n.type);
        merge_cover(cov_children, sub);
    }
    if (n.type == "comment")
        return cov_children;  // комментарии не дают вклада

    if (n.type == "string" && parent_type && *parent_type == "block")
        return {};

    const bool is_docstring = n.type == "expression_statement" && !n.children.empty() &&
                              all_of(n.children.begin(), n.children.end(), [](const Node &child) {
                                  return child.type == "string" || child.type == "comment";
                              });
    if (is_docstring)
        return {};

    // разобьём диапазон узла по строкам и вычтем покрытие детей на каждой строке
    LineSegs own;  // собственные остатки
    auto span_lines = node_span_per_lines(n);
    for (auto &[ln, seg] : span_lines) {
        auto it = cov_children.find(ln);
        const vector<Seg> *kids = (it == cov_children.end()) ? nullptr : &it->second;
        vector<Seg> residual = kids ? subtract_line_seg(seg, *kids) : vector<Seg>{seg};
        for (auto r : residual)
            add_seg(own, ln, r);
    }

    // Решение «универсально, без белых списков»: узел даёт вклад только если у него есть ненулевой остаток
    // (т.е. есть текст, не объясняемый детьми). Это защищает от рамок (их «текст» — скобки/двоеточия),
    // которые обычно покрывают только края и будут вычитаться детьми.
    bool has_own = false;
    for (auto &[ln, v] : own)
        if (!v.empty()) {
            has_own = true;
            break;
        }

    LineSegs cov = cov_children;
    if (has_own)
        merge_cover(cov, own);
    return cov;
}

// итог: количество строк, где есть «значимый» код
static int count_cover_lines(const LineSegs &cov) {
    int cnt = 0;
    for (auto &[ln, v] : cov)
        if (!v.empty())
            ++cnt;
    return cnt;
}


MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    MetricResult::ValueType result = 0;

    if (f.ast.empty())
        return result;

    pa::Node root = pa::Parse(f.ast);
    if (root.type.empty())
        return result;

    const pa::Node *target = pa::FindChild(root, "block");
    if (target == nullptr)
        target = &root;

    auto cov = covered_segs(*target);

    unordered_set<int> docstring_lines;
    unordered_set<int> comment_lines;
    unordered_set<int> structural_only_lines;
    collect_special_lines(*target, nullptr, docstring_lines, comment_lines, structural_only_lines);

    bool file_ok = false;
    auto file_lines = ReadFileLines(f.filename, file_ok);
    if (file_ok && !file_lines.empty()) {
        const int start_line = max(0, target->start.line);
        const int end_line = min(static_cast<int>(file_lines.size()) - 1, target->end.line);
        for (int line = start_line; line <= end_line; ++line) {
            if (docstring_lines.count(line))
                continue;
            if (comment_lines.count(line))
                continue;
            string_view sv(file_lines[line]);
            if (!HasNonWhitespace(sv))
                continue;
            if (structural_only_lines.count(line))
                continue;
            auto it = cov.find(line);
            if (it == cov.end() || it->second.empty())
                add_seg(cov, line, {0, 1});
        }
    }

    result = count_cover_lines(cov);

    return result;
}

std::string CodeLinesCountMetric::Name() const { return "code_lines_count"; }

}  // namespace analyzer::metric::metric_impl
