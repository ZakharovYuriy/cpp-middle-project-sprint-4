#pragma once

#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

namespace analyzer::cmd {

class ProgramOptions {
public:
    ProgramOptions();
    ~ProgramOptions();

    bool Parse(int argc, char *argv[]);

    const std::vector<std::string> &GetFiles() const { return files_; }

private:
    std::vector<std::string> files_;
    boost::program_options::options_description desc_;
};

}  // namespace analyzer::cmd
