#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <bench_util/bench_util.h>
#include <fst/json/json.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>

//#include "gason.h"
#include <fst/print.h>
#include <cstdio>


int main(int argc, const char* argv[])
{
    const char* filepath = RESOURCES_PATH "/multi.json";
//    const char* filepath = RESOURCES_PATH "/canada.json";
    
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
        bench::stop("rapidjson");
    }
    
    bench::start();
    fst::file_buffer file(filepath);
    fst::json::document doc(file.content());
    bench::stop("parse");
    
    bench::start();
    fst::file_buffer file2(filepath);
    unsigned long kkk = 0;
    for(int i = 0; i < 20; i++) {
        for(auto& n : file2.content()) {
            kkk += (int)n;
        }
    }
    
    bench::stop("for loop");
    
        std::cout << doc.to_string() << std::endl;
    
    
    return 0;
}
