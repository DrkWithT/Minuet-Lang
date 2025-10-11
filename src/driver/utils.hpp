#ifndef MINUET_DRIVER_UTILS_HPP
#define MINUET_DRIVER_UTILS_HPP

#include <cstdint>
#include <filesystem>

namespace Minuet::Driver::Utils {
    struct PendingSource {
        std::filesystem::path file_path;
        uint32_t src_id;
    };
}

#endif
