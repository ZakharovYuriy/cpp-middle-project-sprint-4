#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

namespace analyzer::file {

struct File {
    static inline const std::string command_prefix =
        "tree-sitter parse --config-path /root/.config/tree-sitter/config.json ";
    File(const std::string &filename);
    std::string name;
    std::string ast;
    std::vector<std::string> source_lines;

private:
    std::vector<std::string> ReadSourceFile(std::ifstream &file);
    std::string GetAst(const std::string &filename);
};

}  // namespace analyzer::file
