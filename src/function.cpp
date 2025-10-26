#include "function.hpp"

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

#include "file.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;
namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyzer::function {

std::vector<Function> FunctionExtractor::Get(const analyzer::file::File &file) {
    std::vector<Function> functions;
    size_t start = 0;
    const std::string marker = "(function_definition";
    const std::string &ast = file.ast;

    while ((start = ast.find(marker, start)) != std::string::npos) {
        size_t open_braces = 1;
        size_t end = start + marker.length();

        while (end < ast.size() && open_braces > 0) {
            if (ast[end] == '(')
                open_braces++;
            else if (ast[end] == ')')
                open_braces--;
            end++;
        }

        auto func_ast = ast.substr(start, end - start);
        auto name_loc = GetNameLocation(func_ast);
        std::string func_name = GetNameFromSource(func_ast, file.source_lines);

        Function func{.filename = file.name, .class_name = std::nullopt, .name = func_name, .ast = func_ast};

        auto class_info = FindEnclosingClass(ast, name_loc);
        if (class_info) {
            func.class_name = GetClassNameFromSource(*class_info, file.source_lines);
        }

        functions.push_back(func);
        start = end;
    }

    return functions;
}

FunctionExtractor::FunctionNameLocation FunctionExtractor::GetNameLocation(const std::string &function_ast) {
    size_t id_pos = function_ast.find("(identifier");
    if (id_pos == std::string::npos)
        return {};

    size_t coord_start = function_ast.find('[', id_pos);
    size_t coord_end = function_ast.find(']', coord_start);
    std::string coords = function_ast.substr(coord_start + 1, coord_end - coord_start - 1);

    size_t comma = coords.find(',');
    Position start{static_cast<size_t>(ToInt(coords.substr(0, comma))),
                   static_cast<size_t>(ToInt(coords.substr(comma + 2)))};

    size_t dash = function_ast.find('[', coord_end);
    size_t end_bracket = function_ast.find(']', dash);
    std::string end_coords = function_ast.substr(dash + 1, end_bracket - dash - 1);

    comma = end_coords.find(',');
    Position end{static_cast<size_t>(ToInt(end_coords.substr(0, comma))),
                 static_cast<size_t>(ToInt(end_coords.substr(comma + 2)))};

    return {start, end, ""};
}

std::string FunctionExtractor::GetNameFromSource(const std::string &function_ast,
                                                 const std::vector<std::string> &lines) {
    auto loc = GetNameLocation(function_ast);
    if (loc.start.line >= lines.size())
        return "unknown";

    const std::string &target_line = lines[loc.start.line];
    if (loc.start.col >= target_line.size())
        return "unknown";

    return target_line.substr(loc.start.col, loc.end.col - loc.start.col);
}

std::optional<FunctionExtractor::ClassInfo>
FunctionExtractor::FindEnclosingClass(const std::string &ast, const FunctionNameLocation &func_loc) {
    size_t class_pos = 0;
    const std::string class_marker = "(class_definition";
    std::optional<ClassInfo> last_enclosing_class;

    while ((class_pos = ast.find(class_marker, class_pos)) != std::string::npos) {
        size_t coord_start = ast.find('[', class_pos);
        size_t coord_end = ast.find(']', coord_start);
        std::string coords = ast.substr(coord_start + 1, coord_end - coord_start - 1);

        size_t comma = coords.find(',');
        Position class_start{static_cast<size_t>(ToInt(coords.substr(0, comma))),
                             static_cast<size_t>(ToInt(coords.substr(comma + 1)))};

        size_t dash = ast.find('-', coord_end);
        size_t second_coord_start = ast.find('[', dash);
        size_t second_coord_end = ast.find(']', second_coord_start);
        std::string end_coords = ast.substr(second_coord_start + 1, second_coord_end - second_coord_start - 1);

        comma = end_coords.find(',');
        Position class_end{static_cast<size_t>(ToInt(end_coords.substr(0, comma))),
                           static_cast<size_t>(ToInt(end_coords.substr(comma + 1)))};

        if (func_loc.start.line > class_start.line ||
            (func_loc.start.line == class_start.line && func_loc.start.col >= class_start.col)) {
            if (func_loc.start.line < class_end.line ||
                (func_loc.start.line == class_end.line && func_loc.start.col <= class_end.col)) {
                size_t name_start = ast.find("name:", coord_end);
                if (name_start != std::string::npos) {
                    size_t id_start = ast.find("(identifier", name_start);
                    if (id_start != std::string::npos) {
                        size_t id_coord_start = ast.find('[', id_start);
                        size_t id_coord_end = ast.find(']', id_coord_start);
                        std::string id_coords = ast.substr(id_coord_start + 1, id_coord_end - id_coord_start - 1);

                        ClassInfo class_info;
                        class_info.start = class_start;
                        class_info.end = class_end;
                        last_enclosing_class = class_info;
                    }
                }
            }
        }

        class_pos = second_coord_end;
    }

    return last_enclosing_class;
}

std::string FunctionExtractor::GetClassNameFromSource(const ClassInfo &class_info,
                                                      const std::vector<std::string> &lines) {
    if (class_info.start.line >= lines.size())
        return "unknown";

    const std::string &class_line = lines[class_info.start.line];

    size_t class_pos = class_line.find("class");
    if (class_pos == std::string::npos)
        return "unknown";

    size_t name_start = class_line.find_first_not_of(" \t", class_pos + 5);
    if (name_start == std::string::npos)
        return "unknown";

    size_t name_end = class_line.find_first_of(" :{(", name_start);
    if (name_end == std::string::npos)
        name_end = class_line.length();

    return class_line.substr(name_start, name_end - name_start);
}

}  // namespace analyzer::function
