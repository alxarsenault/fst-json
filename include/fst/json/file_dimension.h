#pragma once

#include <cstdint>

namespace fst {
namespace json {
    enum class file_dimension : std::uint8_t { tiny, small, medium, big, huge };

    enum file_dimension_size {
        fds_tiny = 100,
        fds_small = 500,
        fds_medium = 1000,
        fds_big = 10000
    };

    enum file_dimension_reserve {
        fdr_tiny = 20,
        fdr_small = 100,
        fdr_medium = 500,
        fdr_big = 1000,
        fdr_huge = 500000
    };

    file_dimension get_file_dimension_from_size(std::size_t size)
    {
        if (size < fds_tiny) {
            return file_dimension::tiny;
        } else if (size < fds_small) {
            return file_dimension::small;
        } else if (size < fds_medium) {
            return file_dimension::medium;
        } else if (size < fds_big) {
            return file_dimension::big;
        }

        return file_dimension::huge;
    }

    std::size_t get_reserve_size_from_dimension(file_dimension fs)
    {
        switch (fs) {
        case file_dimension::tiny:
            return fdr_tiny;
        case file_dimension::small:
            return fdr_small;
        case file_dimension::medium:
            return fdr_medium;
        case file_dimension::big:
            return fdr_big;
        case file_dimension::huge:
            return fdr_huge;
        }
    }

} // json.
} // fst.
