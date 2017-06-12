#include <cstdio>
#include <cstdlib>

std::size_t new_count = 0;
// replacement of a minimal set of functions:
void* operator new(std::size_t sz)
{
    new_count++;
    return std::malloc(sz);
}

#include <fst/json/document.h>
#include <fst/print.h>

int main()
{
    const char* filepath = RESOURCES_PATH "/canada.json";
    fst::file_buffer file(filepath);
    fst::buffer_view<char> data = file.content();
    fst::json::document doc(data);
    fst::print("New count", (int)new_count);
    return 0;
}
