#pragma once

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "domain.h"

#include <memory>
#include <string>
#include <variant>
#include <algorithm>

/*
 * Здесь код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace in {
    using namespace json;
    using namespace std::literals;
class JsonReader {
    const std::string BASE_REQS = "base_requests"s;
    const std::string STAT_REQS = "stat_requests"s;
    const std::string RENDER_SETTINGS = "render_settings"s;
    const std::string ROUTINGS_SETTINGS = "routing_settings"s;
    const std::string SERIALIZATION_SETTINGS = "serialization_settings"s;

    struct RouteItemsPrinter {
        Node operator()(std::monostate) const {
            return Node(Dict{});
        }
        Node operator()(router::WaitItem wait) const {
            return Node(Builder{}.StartDict()
                .Key("type"s).Value("Wait"s)
                .Key("stop_name"s).Value(std::string(wait.stop_name))
                .Key("time"s).Value(wait.time)
                .EndDict().Build());
        }
        Node operator()(router::BusItem bus) const {
            return Node(Builder{}.StartDict()
                .Key("type"s).Value("Bus"s)
                .Key("bus"s).Value(std::string(bus.bus_name))
                .Key("span_count"s).Value(bus.span_count)
                .Key("time"s).Value(bus.time)
                .EndDict().Build());
        }
    };

    struct StatsPrinter {
        std::ostream& out = std::cout;
        int id_ = 0;

        Node operator()(std::monostate) const {
            return Node(Dict{});
        }
        Node operator()(std::optional<RouteStats> route) const {
            Node response{};
            if (route) {
                response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                    .Key("curvature"s).Value((*route).curvative)
                    .Key("route_length"s).Value((*route).route_length)
                    .Key("stop_count"s).Value((int)(*route).stop_count)
                    .Key("unique_stop_count"s).Value((int)(*route).unique_stop_count)
                    .EndDict().Build();
            }
            else {
                response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict().Build();
            }
            return response;
        }
        Node operator()(std::optional<StopBuses> buses) const {
            std::vector<std::string> buses_list{};
            Node response{};
            if (buses) {
                if (buses.value()) {
                    for (auto bus : *buses.value()) {
                        buses_list.push_back(Node(bus->name).AsString());
                        std::sort(buses_list.begin(), buses_list.end());
                    }
                    Array sorted(std::begin(buses_list), std::end(buses_list));
                    response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                        .Key("buses"s).Value(sorted)
                        .EndDict().Build();
                }
                else {
                    response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                        .Key("buses"s).Value(Array{})
                        .EndDict().Build();
                }
            }
            else {
                response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict().Build();
            }
            return response;
        }
        Node operator()(std::shared_ptr<svg::Document> map) const {
            Node response{};
            std::ostringstream svg_stream;
            map->Render(svg_stream);
            response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                .Key("map"s).Value(svg_stream.str())
                .EndDict().Build();
            return response;
        }
        Node operator()(std::optional<router::OptimalRoute> route) const {
            Node response{};
            Array items{};
            if (route == std::nullopt) {
                response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict().Build();
            }
            else {
                for (auto item : (*route).items) {
                    items.push_back(std::visit(RouteItemsPrinter{}, item));
                }
                response = Builder{}.StartDict().Key("request_id"s).Value(id_)
                    .Key("total_time"s).Value((*route).total_time)
                    .Key("items"s).Value(items)
                    .EndDict().Build();
            }
            return response;
        }
    };

public:
    explicit JsonReader(RequestHandler& req_handler);

    void ReadDocument(std::istream& input);
    void PrintStatsRequests(std::ostream& output) const;

private:
    void ReadStop(Array base_reqs);
    void ReadBus(Array base_reqs);
    void ReadStatReqs(Array stat_reqs);
    void ReadRenderSettings(Dict base_reqs) const;
    void ReadRoutingSettings(Dict json) const;
    void ReadSerializationSettings(Dict serialize_req);
    RequestHandler& req_handler_;
};

} // namespace in