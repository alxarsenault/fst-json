#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <bench_util/bench_util.h>
#include <fst/json/document.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>

#include <fst/print.h>
#include <cstdio>

int main()
{
    const char* filepath = RESOURCES_PATH "/big.json";
    // const char* filepath = RESOURCES_PATH "/canada.json";
    // const char* filepath = RESOURCES_PATH "/super.json";

    bench::start();
    fst::file_buffer file2(filepath);
    unsigned long kkk = 0;
    for (int i = 0; i < 20; i++) {
        for (auto& n : file2.content()) {
            kkk += (int)n;
        }
    }

    bench::stop("for loop");

    double rpj;
    double fstj;
    // rapidjson.
    {
        bench::start();
        FILE* fp = fopen(filepath, "rb");
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET); // same as rewind(f);
        char* readBuffer = (char*)malloc(fsize);
        rapidjson::FileReadStream iss(fp, readBuffer, fsize);
        rapidjson::Document d;
        d.ParseStream(iss);
        rpj = bench::stop("rapidjson");
    }

    bench::start();
    fst::file_buffer file(filepath);
    fst::json::document doc(file.content());
    fstj = bench::stop("parse");

    fst::print("fst :", fstj / rpj, "%");
    fst::print("rapidjson :", rpj / fstj, "%");
    // std::cout << doc.to_string() << std::endl;

    return 0;
}
