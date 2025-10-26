#include "cmd_options.hpp"

#include <exception>
#include <iostream>
#include <print>
#include <string>

#include <boost/program_options.hpp>

namespace analyzer::cmd {

namespace po = boost::program_options;

ProgramOptions::ProgramOptions() : desc_("Allowed options") {
    desc_.add_options()("help,h", "Display help message")(
        "file,f", po::value<std::vector<std::string>>(&files_)->required()->multitoken(),
        "List of files to process (required)");
}

ProgramOptions::~ProgramOptions() = default;

bool ProgramOptions::Parse(int argc, char *argv[]) {
    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc_).run(), vm);

        if (vm.count("help")) {
            desc_.print(std::cout);
            return false;
        }

        po::notify(vm);

        if (files_.empty()) {
            std::cerr << "Error: At least one file must be specified\n";
            desc_.print(std::cout);
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing command line: " << e.what() << "\n";
        desc_.print(std::cout);
        return false;
    }
}

}  // namespace analyzer::cmd
