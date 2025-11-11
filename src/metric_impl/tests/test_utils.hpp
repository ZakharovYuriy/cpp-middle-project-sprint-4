#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::tests {

inline std::filesystem::path SamplesDir(const char *anchor_file) {
    return std::filesystem::path(anchor_file).parent_path() / "files";
}

inline std::filesystem::path SamplePath(const char *anchor_file, std::string_view filename) {
    return SamplesDir(anchor_file) / std::filesystem::path(std::string(filename));
}

inline analyzer::function::Function LoadFunction(std::string_view function_name,
                                                 const std::filesystem::path &sample_path) {
    analyzer::file::File file(sample_path.string());
    analyzer::function::FunctionExtractor extractor;
    auto functions = extractor.Get(file);
    const auto it = std::find_if(functions.begin(), functions.end(),
                                 [&](const analyzer::function::Function &func) {
                                     return func.name == function_name;
                                 });
    if (it == functions.end()) {
        throw std::runtime_error("Function " + std::string(function_name) + " not found in " + sample_path.string());
    }
    return *it;
}

inline std::string SanitizeName(std::string value) {
    for (auto &ch : value)
        if (!std::isalnum(static_cast<unsigned char>(ch)))
            ch = '_';
    return value;
}

inline std::string ComposeParamName(std::string_view filename, std::string_view function_name) {
    const std::string base = std::filesystem::path(std::string(filename)).stem().string();
    return SanitizeName(base + "_" + std::string(function_name));
}

}  // namespace analyzer::metric::tests
