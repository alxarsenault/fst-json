#include <fst/json/document.h>
#include <fst/print.h>
#include <iostream>

int main()
{
    fst::file_buffer file(RESOURCES_PATH "/big.json");
    fst::buffer_view<char> data = file.content();
    fst::json::document doc(data);

    std::cout << doc["widget"]["window"]["title"].value() << std::endl;
    std::cout << doc["widget"]["window"]["width"].convert<int>() << std::endl;
    std::cout << doc["widget"]["window"]["height"].convert<int>() << std::endl;
    std::cout << doc["widget"]["text"]["onMouseUp"].value() << std::endl;
    return 0;
}
