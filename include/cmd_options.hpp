#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

namespace analyzer::cmd {

class ProgramOptions {
public:
    ProgramOptions();
    ~ProgramOptions();

    bool Parse(int argc, char *argv[]);

    const std::vector<std::string> &GetFiles() const { return files_; }
    bool IsHelpRequested() const { return help_requested_; }
    bool DebugEnabled() const { return debug_enabled_; }

private:
    std::vector<std::string> files_;
    boost::program_options::options_description desc_;
    bool help_requested_ = false;
    bool debug_enabled_ = false;
};

}  // namespace analyzer::cmd
