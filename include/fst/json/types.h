#pragma once

#include <cstdint>

namespace fst {
namespace json {
    enum class type : std::uint8_t {
        null,
        object,
        array,
        string,
        boolean,
        number,
        error
    };
} // json.
} // fst.
