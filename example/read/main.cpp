#include <fst/json/document.h>
#include <fst/print.h>
#include <iostream>

#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <bench_util/bench_util.h>
#include <fst/json/document.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>

#include <cstdio>

int main()
{
    const char* filepath = RESOURCES_PATH "/big.json";
    fst::file_buffer file(filepath);
    fst::buffer_view<char> data = file.content();
    fst::json::document doc(data);

    FILE* fp = fopen(filepath, "rb");
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET); // same as rewind(f);
    char* readBuffer = (char*)malloc(fsize);
    rapidjson::FileReadStream iss(fp, readBuffer, fsize);
    rapidjson::Document d;
    d.ParseStream(iss);

    std::vector<std::string> data0;
    std::vector<std::string> data1;
    data0.reserve(500000 * 2);
    data1.reserve(500000 * 2);

    bench::start();
    for (int i = 0; i < 500000; i++) {
        auto& n0 = d["widget"]["window"]["title"];
        data1.emplace_back(std::string(n0.GetString(), n0.GetStringLength()));
        auto& n1 = d["widget"]["text"]["onMouseUp"];
        data1.emplace_back(std::string(n1.GetString(), n1.GetStringLength()));
    }
    bench::stop("rapidjson");

    bench::start();
    for (int i = 0; i < 500000; i++) {
        data0.emplace_back(doc["widget"]["window"]["title"].value().to_string());
        data0.emplace_back(doc["widget"]["text"]["onMouseUp"].value().to_string());
    }
    bench::stop("fst");

    std::cout << doc["widget"]["window"]["title"].value() << std::endl;
    std::cout << doc["widget"]["window"]["width"].convert<int>() << std::endl;
    std::cout << doc["widget"]["window"]["height"].convert<int>() << std::endl;
    std::cout << doc["widget"]["text"]["onMouseUp"].value() << std::endl;

    return 0;
}
