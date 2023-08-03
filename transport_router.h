#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <utility>
#include <vector>
#include <set>
#include <variant>
#include <string_view>
#include <memory>
#include <optional>
#include <functional>

namespace ptb { class Protobuffer; }

namespace router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct WaitItem {
    std::string stop_name;
    int time = 0;
};

struct BusItem {
    std::string bus_name;
    int span_count = 0;
    double time = 0.0;
};

using RouteItem = std::variant<std::monostate, WaitItem, BusItem>;

struct OptimalRoute {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

class TransportRouter {
private:
    friend ptb::Protobuffer;
public:
    TransportRouter(const transport_db::TransportCatalogue& db);
    TransportRouter(const transport_db::TransportCatalogue& db ,
                    RoutingSettings settings);
    void InitGraph();
    void InitRouter();
    std::optional<OptimalRoute> GetOptimalRoute(std::string_view from, std::string_view to);
    void AddVertexesToGraph();
    void AddEdgesToGraph();

private:
    std::pair<graph::VertexId, graph::VertexId> GetStopVertexId(const ::Stop* stop);
    OptimalRoute RouterResultParser(graph::Router<double>::RouteInfo route);
    inline double KmphToMpm(double kmph);
    graph::EdgeId AddEdge(graph::VertexId from, graph::VertexId to, double distance);
    void AddWaitEdge(graph::VertexId from, graph::VertexId to, const Stop* stop);
    std::pair<graph::VertexId, graph::VertexId> AddVertexId(const ::Stop* stop);
    std::pair<graph::VertexId, graph::VertexId> GetVertexId(const ::Stop* stop);
    template<typename It>
    void AddEdgesFromBus(It from, It to, const Bus* bus);

    graph::VertexId last_vertex_id_ = 0;
    const transport_db::TransportCatalogue& db_;

    RoutingSettings settings_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    std::unordered_map<
        const ::Stop*,
        std::pair<graph::VertexId, graph::VertexId>> stop_to_vertex_id_;
    std::unordered_map<
        graph::EdgeId,
        RouteItem> edge_id_to_item_;
};

template<typename It>
void TransportRouter::AddEdgesFromBus(It from, It to, const Bus* bus) {
    using namespace graph;
    for (auto from_ = from; from_ != std::prev(to); ++from_) {
        double accumulated_weight = 0.;
        int spans_count = 0;

        for (auto to_ = std::next(from_); to_ != to; ++to_) {
            spans_count += 1;
            accumulated_weight += db_.GetStopsDistance({ *std::prev(to_), *to_ }) / KmphToMpm(settings_.bus_velocity);
            auto [_, v_from] = GetVertexId(*from_);
            auto [v_to, __] = GetVertexId(*to_);
            EdgeId id = AddEdge(v_from, v_to, accumulated_weight);
            edge_id_to_item_[id] = BusItem{ bus->name, spans_count, accumulated_weight };
        }
    }
}

} // route