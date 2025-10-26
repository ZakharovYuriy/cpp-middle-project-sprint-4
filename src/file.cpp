#include "file.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

namespace analyzer::file {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

File::File(const std::string &filename) : name{filename} {
    std::ifstream file(name);

    if (!file.is_open()) {
        throw std::invalid_argument("Can't open file " + filename);
    }
    ast = GetAst(filename);
    source_lines = ReadSourceFile(file);
}

std::vector<std::string> File::ReadSourceFile(std::ifstream &file) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string File::GetAst(const std::string &filename) try {
    std::string full_cmd = File::command_prefix + filename + " 2>&1";
    std::string result;
    std::array<char, 256> buffer;

    using PipePtr = std::unique_ptr<FILE, decltype([](FILE *pipe) {
                                        if (!pipe)
                                            return;

                                        int status = pclose(pipe);
                                        if (WIFEXITED(status)) {
                                            int exit_status = WEXITSTATUS(status);
                                            if (exit_status != 0) {
                                                throw std::runtime_error("Command failed with exit code " +
                                                                         std::to_string(exit_status));
                                            }
                                        } else {
                                            throw std::runtime_error("Command terminated abnormally");
                                        }
                                    })>;

    FILE *raw_pipe = popen(full_cmd.c_str(), "r");
    if (!raw_pipe) {
        throw std::runtime_error("Failed to execute command: " + std::string(std::strerror(errno)));
    }
    PipePtr pipe(raw_pipe);

    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        result += buffer.data();
    }

    return result;
} catch (const std::exception &e) {
    throw std::runtime_error("Error while getting ast from " + filename);
}

}  // namespace analyzer::file
