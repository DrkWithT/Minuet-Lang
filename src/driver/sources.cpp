#include <iostream>
#include <print>
#include <sstream>
#include <fstream>

namespace Minuet::Driver::Sources {
    auto read_source(std::filesystem::path src_path) -> std::string {
        if (!src_path.has_filename()) {
            std::println(std::cerr, "Invalid src_path {}- no filename found!", src_path.c_str());

            return {};
        }

        std::ifstream reader {src_path};

        if (!reader.is_open()) {
            std::println("Failed to read source at '{}'", src_path.c_str());

            return {};
        }

        std::ostringstream sout;
        std::string temp_line;

        while (std::getline(reader, temp_line)) {
            sout << temp_line << '\n';
            temp_line.clear();
        }

        return sout.str();
    }
}
