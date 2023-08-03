#include "transport_router.h"

using namespace std;

namespace router {

TransportRouter::TransportRouter(const transport_db::TransportCatalogue& db)
	: db_(db)
{
	graph_ = make_unique<graph::DirectedWeightedGraph<double>>(0);
	router_ = make_unique<graph::Router<double>>(*graph_);
}

TransportRouter::TransportRouter(const transport_db::TransportCatalogue& db,
								 RoutingSettings settings)
	:	db_(db),
		settings_(settings)
		//graph_(db_.GetStops().size() * 2)
{
	//InitRouter();
}

void TransportRouter::InitGraph() {
	graph_ = make_unique<graph::DirectedWeightedGraph<double>>(db_.GetStops().size() * 2);
}

void TransportRouter::InitRouter() {
	AddVertexesToGraph();
	AddEdgesToGraph();

	router_ = make_unique<graph::Router<double>>(*graph_);
}

void TransportRouter::AddVertexesToGraph() {
	const auto& all_stops = db_.GetStops();
	// пронумеруем остановки каждого маршрута
	for (const ::Stop& stop : all_stops) {
		auto [from, to] = AddVertexId(&stop);
		AddWaitEdge(from, to, &stop);
	}
}

void TransportRouter::AddEdgesToGraph() {
	// добавляем рёбра между остановками для каждого маршрута
	const auto& all_buses = db_.GetBuses();
	for (const auto& bus : all_buses) {
		AddEdgesFromBus(bus.route.begin(), bus.route.end(), &bus);
		if (bus.is_roundtrip == false) {
			AddEdgesFromBus(bus.route.rbegin(), bus.route.rend(), &bus);
		}
	}
}

void TransportRouter::AddWaitEdge(graph::VertexId from, graph::VertexId to, const Stop* stop) {
	auto item = WaitItem{ stop->name, settings_.bus_wait_time };
	graph::EdgeId id = AddEdge(from, to, (double)settings_.bus_wait_time);
	edge_id_to_item_[id] = item;
}

graph::EdgeId TransportRouter::AddEdge(graph::VertexId from, graph::VertexId to, double time) {
	graph::Edge<double> graph_edge = { from, to, time };
	return graph_->AddEdge(graph_edge);
}

pair<graph::VertexId, graph::VertexId> TransportRouter::AddVertexId(const ::Stop* stop) {
	pair<graph::VertexId, graph::VertexId> pair;
	pair.first = last_vertex_id_;
	pair.second = ++last_vertex_id_;
	stop_to_vertex_id_[stop] = pair;
	++last_vertex_id_;
	return stop_to_vertex_id_.at(stop);
}

pair<graph::VertexId, graph::VertexId> TransportRouter::GetVertexId(const ::Stop* stop) {
	return stop_to_vertex_id_.at(stop);
}

optional<OptimalRoute> TransportRouter::GetOptimalRoute(string_view from, string_view to) {
	using namespace graph;
	using namespace transport_db;

	auto [from_, _] = GetStopVertexId(db_.FindStop(from));
	auto [to_, __] = GetStopVertexId(db_.FindStop(to));
	vector<Router<double>::RouteInfo> routes;

	auto route = router_->BuildRoute(from_, to_);

	if (!route) {
		return nullopt;
	}

	return RouterResultParser(move(route.value()));
}

OptimalRoute TransportRouter::RouterResultParser(graph::Router<double>::RouteInfo route) {
	OptimalRoute optimal_route;
	if (route.edges.empty()) {
		return optimal_route;
	}

	// основной маршрут
	for (const auto& edge_id : route.edges) {
		optimal_route.items.push_back(edge_id_to_item_.at(edge_id));
	}
	optimal_route.total_time = route.weight;

	return optimal_route;
}

pair<graph::VertexId, graph::VertexId> TransportRouter::GetStopVertexId(const ::Stop* stop) {
	return stop_to_vertex_id_.at(stop);
}

// конвертер в минуты
inline double TransportRouter::KmphToMpm(double kmph) {
	return kmph * 1000 / 60;
}

}