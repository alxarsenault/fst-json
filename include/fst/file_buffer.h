#pragma once

#include <fst/buffer_view.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace fst {
class file_buffer {
public:
    inline file_buffer(const char* filepath)
    {
        // Create the file mapping.
        boost::interprocess::file_mapping fm(
            filepath, boost::interprocess::read_only);

        // Map the file in memory.
        _mapped_region = boost::interprocess::mapped_region(
            fm, boost::interprocess::read_only);

        // Create file data mutable buffer.
        _content = fst::buffer_view<char>(
            (char*)_mapped_region.get_address(), _mapped_region.get_size());
    }

    inline fst::buffer_view<char> content() const { return _content; }

private:
    boost::interprocess::mapped_region _mapped_region;
    fst::buffer_view<char> _content;
};
} // fst.
