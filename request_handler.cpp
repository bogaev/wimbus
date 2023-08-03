#include "request_handler.h"

using namespace std;
using namespace in;

RequestHandler::RequestHandler(transport_db::TransportCatalogue& db)
	:	db_(db)
{
	stops_requests_.reserve(1000);
}

RequestHandler::RequestHandler(transport_db::TransportCatalogue& db,
	MapRenderer* map_renderer)
	: db_(db),
	map_renderer_(map_renderer)
{
	stops_requests_.reserve(1000);
}

void RequestHandler::AddStopRequest(Stop stop) {
	stops_requests_.push_back(move(stop));
}

void RequestHandler::AddBusRequest(BusRoute bus) {
	buses_requests_.push_back(move(bus));
}

void RequestHandler::AddStatRequest(StatRequest req) {
	stat_requests_.push_back(move(req));
}

void RequestHandler::AddRenderSettings(RenderSettings settings) {
	render_settings_ = move(settings);
}

void RequestHandler::AddRoutingSettings(router::RoutingSettings settings) {
	routing_settings_ = move(settings);
}

void RequestHandler::AddSerializeSettings(string settings) {
	serialize_settings_ = move(ptb::Settings{settings});
}

void RequestHandler::ProcessBaseCreateRequests() {
	ProcessStopRequests();
	ProcessBusRequests();
	if (routing_settings_.has_value()) {
		router_ = make_unique<router::TransportRouter>(db_, routing_settings_.value());
		router_->InitGraph();
		router_->InitRouter();
	}
}

const std::vector<Response>& RequestHandler::GetResponses() {
	return responses_;
}

optional<RouteStats> RequestHandler::GetBusStat(const string_view& bus_name) const {
	optional<RouteStats> ret;
	BusPtr bus = db_.FindBus(bus_name);
	if (bus) {
		RouteStats route_stats;
		if (!bus->is_roundtrip) {
			Bus tmp(*bus);
			tmp.route.insert(tmp.route.end(), (tmp.route.rbegin() + 1), tmp.route.rend());
			double geo_length = db_.ComputeGeoRouteDistance(&tmp);
			route_stats.route_length = db_.ComputeRealRouteDistance(&tmp);
			route_stats.stop_count = tmp.route.size();
			route_stats.curvative = route_stats.route_length / geo_length;
		}
		else {
			double geo_length = db_.ComputeGeoRouteDistance(bus);
			route_stats.route_length = db_.ComputeRealRouteDistance(bus);
			route_stats.stop_count = bus->route.size();
			route_stats.curvative = route_stats.route_length / geo_length;
		}

		auto raw_route(bus->route);
		sort(raw_route.begin(), raw_route.end());
		auto last = unique(raw_route.begin(), raw_route.end());
		vector<const Stop*> unique_stops(raw_route.begin(), last);
		route_stats.unique_stop_count = unique_stops.size();

		ret = route_stats;
	}

	return ret;
}

AllBusesPtr RequestHandler::GetAllBuses() const {
	return db_.GetAllBuses();
}

optional<StopBuses> RequestHandler::GetBusesByStop(const string_view& stop_name) const {
	return db_.GetBusesNamesByStop(stop_name);
}

void RequestHandler::ProcessStopRequests() {
	for (const auto& stop : stops_requests_) {
		db_.AddStop(stop);
	}
	for (const auto& stop : stops_requests_) {
		for (const auto& distance : stop.road_distances) {
			db_.SetStopsDistance(stop.name, distance.first, distance.second);
		}
	}
	stops_requests_.clear();
}

void RequestHandler::ProcessBusRequests() {
	for (const auto& bus : buses_requests_) {
		db_.AddBus(bus.name, bus.stops, bus.is_roundtrip);
	}
	buses_requests_.clear();
}

void RequestHandler::ProcessStatRequests() {
	for (auto req : stat_requests_) {
		if (req.type == enStatRequestsType::BUS) {
			responses_.push_back({ req.id, GetBusStat(move(req.name)) });
		}
		else if (req.type == enStatRequestsType::STOP) {
			responses_.push_back({ req.id, GetBusesByStop(move(req.name)) });
		}
		else if (req.type == enStatRequestsType::MAP) {
			if (map_renderer_ == nullptr) {
				map_renderer_ = make_unique<MapRenderer>();
			}
			auto map = make_shared<svg::Document>(RenderMap(GetAllBuses()));
			responses_.push_back({ req.id, move(map) });
		}
		else if (req.type == enStatRequestsType::ROUTE) {
			responses_.push_back({ req.id, move(router_->GetOptimalRoute(move(req.from), move(req.to))) });
		}
	}
	stat_requests_.clear();
}

void RequestHandler::SerializeBase() {
	if (serialize_settings_.has_value()) {
		auto ms = render_settings_ ? &render_settings_.value() : nullptr;
		auto rs = routing_settings_ ? &routing_settings_.value() : nullptr;
		ptb::Protobuffer ptb(db_, ms, rs, router_.get());
		ptb.SerializeDB(serialize_settings_.value().filename);
	}
}

void RequestHandler::DeserializeBase() {
	if (serialize_settings_.has_value()) {
		RenderSettings render_settings;
		render_settings.Clear();
		render_settings_ = render_settings;
		auto ms = &render_settings_.value();

		router::RoutingSettings routing_settings;
		routing_settings_ = routing_settings;
		auto rs = &routing_settings_.value();

		router_ = make_unique<router::TransportRouter>(db_);

		ptb::Protobuffer ptb(db_, ms, rs, router_.get());
		ptb.DeserializeDB(serialize_settings_.value().filename);
	}
}
