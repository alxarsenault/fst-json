#include <fst/json/document.h>
#include <fst/print.h>
#include <iostream>

int main()
{
    //    const char* filepath = RESOURCES_PATH "/string_array.json";
    const char* filepath = RESOURCES_PATH "/slash.json";
    fst::file_buffer file(filepath);
    fst::buffer_view<char> data = file.content();
    fst::json::document doc(data);

    std::cout << doc.to_string() << std::endl;

    //    for(auto n : doc["string_array"]) {
    //        std::cout << n.name() << " " << n.value() << std::endl;
    //    }
    //    std::cout << doc["string_array"]
    return 0;
}
