#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

#include <transport_catalogue.pb.h>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        transport_db::TransportCatalogue db;
        in::RequestHandler req_handler(db);
        in::JsonReader json(req_handler);
        json.ReadDocument(std::cin);
        req_handler.ProcessBaseCreateRequests();
        req_handler.SerializeBase();
    }
    else if (mode == "process_requests"sv) {
        // process requests here
        transport_db::TransportCatalogue db;
        in::RequestHandler req_handler(db);
        in::JsonReader json(req_handler);
        json.ReadDocument(std::cin);
        req_handler.DeserializeBase();
        req_handler.ProcessStatRequests();
        json.PrintStatsRequests(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }

    return 0;
}