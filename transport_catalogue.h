#pragma once

#include "geo.h"
#include "domain.h"
#include "ranges.h"

#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <sstream>
#include <optional>

namespace ptb {
	class Protobuffer;
}

namespace transport_db {

namespace detail {
	using StopsDistance = std::pair<std::string, std::string>;
	using StopsPair = std::pair<const Stop*, const Stop*>;
	struct StopsHasher {
		size_t operator() (const StopsPair& key) const {
			return v_hasher_(key.first) + v_hasher_(key.second) * 37;
		}
	private:
		std::hash<const void*> v_hasher_;
	};
}

using SpansMap = std::unordered_map<detail::StopsPair, int, detail::StopsHasher>;
using SpansBusesMap = std::unordered_map<detail::StopsPair, std::vector<BusPtr>, detail::StopsHasher>;

class TransportCatalogue
{
	friend ptb::Protobuffer;
public:

	void AddStop(const Stop& stop);
	void SetStopsDistance(std::string from, std::string to, int distance);
	void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
	const Stop* FindStop(std::string_view stop_name) const;
	BusPtr FindBus(std::string_view bus_name) const;
	std::optional<StopBuses> GetBusesNamesByStop(const std::string_view& stop_name) const;
	AllBusesPtr GetAllBuses() const;
	const std::deque<Stop>& GetStops() const;
	const std::deque<Bus>& GetBuses() const;
	double GetStopsDistance(const detail::StopsPair& stops) const;
	double ComputeRealRouteDistance(BusPtr bus);
	double ComputeGeoRouteDistance(BusPtr bus);

	std::deque<Stop>& GetStops();
	std::deque<Bus>& GetBuses();

private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, BusPtr> busname_to_bus_;
	std::unordered_map<std::string_view, std::set<BusPtr>> stopname_to_buses_;
	SpansMap stops_distance_;
};

}