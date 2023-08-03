#include "transport_catalogue.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

namespace transport_db {

void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(stop);
	Stop& last_added = stops_.back();
	stopname_to_stop_.insert({ last_added.name, &last_added });
	stopname_to_buses_[last_added.name];
}

void TransportCatalogue::SetStopsDistance(string from, string to, int distance) {
	auto stop_from = FindStop(from);
	if (stop_from != nullptr) {
		auto stop_to = FindStop(to);
		if (stop_to != nullptr) {
			detail::StopsPair key{ stop_from, stop_to };
			stops_distance_[key] = distance;
		}
		else {
			//cout << "Next Stop Not Found!" << endl;
		}
	}
	else {
		//cout << "Base Stop Not Found!" << endl;
	}
}

const Stop* TransportCatalogue::FindStop(string_view stop_name) const {
	if (stopname_to_stop_.count(stop_name)) {
		return stopname_to_stop_.at(stop_name);
	}
	return nullptr;
}

void TransportCatalogue::AddBus(const string& name, const vector<string>& stops, bool is_roundtrip) {
	Bus bus;

	bus.name = name;

	for (auto stop : stops) {
		auto s = FindStop(stop);
		if (s != nullptr) {
			bus.route.push_back(s);
		}
		else {
			//cout << "Stop Not Found!" << endl;
		}
	}

	bus.is_roundtrip = is_roundtrip;

	buses_.push_back(move(bus));
	Bus& last_added = buses_.back();
	busname_to_bus_.insert({ last_added.name, &last_added });
	for (auto stop : stops) {
		stopname_to_buses_[stop].insert(&last_added);
	}
}

const Bus* TransportCatalogue::FindBus(string_view bus_name) const {
	if (busname_to_bus_.count(bus_name)) {
		return busname_to_bus_.at(bus_name);
	}
	return nullptr;
}

optional<StopBuses> TransportCatalogue::GetBusesNamesByStop(const string_view& stop_name) const {
	optional<StopBuses> ret;
	if (stopname_to_buses_.count(stop_name)) {
		if (stopname_to_buses_.at(stop_name).size() == 0) {
			ret = nullptr;
			//return nullptr;
		}
		else {
			ret = { &(stopname_to_buses_.at(stop_name)) };
			//return { &(stopname_to_buses_.at(stop_name)) };
		}
	}
	return ret;
}

AllBusesPtr TransportCatalogue::GetAllBuses() const {
	return &busname_to_bus_;
}

double TransportCatalogue::ComputeRealRouteDistance(const Bus* bus) {
	double distance = 0;
	auto route = bus->route;
	for (auto it = next(route.begin()); it < route.end(); ++it) {
		distance += GetStopsDistance({ *prev(it), *it });
	}
	return distance;
}

double TransportCatalogue::ComputeGeoRouteDistance(const Bus* bus) {
	double geo_distance = 0;
	auto route = bus->route;
	for (auto it = next(route.begin()); it < route.end(); ++it) {
		geo_distance += ComputeDistance((*prev(it))->coord, (*it)->coord);
	}
	return geo_distance;
}

double TransportCatalogue::GetStopsDistance(const detail::StopsPair& stops) const {
	if (stops_distance_.count(stops)) {
		return stops_distance_.at(stops);
	}
	else if (stops_distance_.count({ stops.second, stops.first })) {
		return stops_distance_.at({ stops.second, stops.first });
	}
	else {
		//cout << "Distance not found"s << endl;
	}
	return 0;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
	return stops_;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
	return buses_;
}

std::deque<Stop>& TransportCatalogue::GetStops() {
	return stops_;
}

std::deque<Bus>& TransportCatalogue::GetBuses() {
	return buses_;
}

}