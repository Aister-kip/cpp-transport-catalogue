﻿#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase() {
    catalogue::TransportCatalogue db;
    renderer::MapRenderer mp;
    transport_router::TransportRouter tr;
    json_reader::JsonReader input(std::cin, &db, &mp, &tr);
    input.ProcessInput_();
}

void ProcessRequest() {
    catalogue::TransportCatalogue db;
    renderer::MapRenderer mp;
    transport_router::TransportRouter tr;
    json_reader::JsonReader input(std::cin, &db, &mp, &tr);
    input.PrintResult(std::cout);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        MakeBase();
    }
    else if (mode == "process_requests"sv) {
        ProcessRequest();
    }
    else {
        PrintUsage();
        return 1;
    }
}