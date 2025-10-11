#ifndef MINUET_SOURCES_HPP
#define MINUET_SOURCES_HPP

#include <string>
#include <filesystem>

namespace Minuet::Driver::Sources {   
    [[nodiscard]] auto read_source(std::filesystem::path src_path) -> std::string;
}
    
#endif
