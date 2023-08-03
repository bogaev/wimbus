#pragma once

/*
 * В этом файле классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 */

#include "geo.h"
#include "svg.h"

#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <variant>
#include <optional>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

struct RouteStats {
	double curvative = 0;
	double route_length = 0;
	size_t stop_count = 0;
	size_t unique_stop_count = 0;
};

struct Stop {
	std::string name;
	geo::Coordinates coord;
	std::unordered_map<std::string, int> road_distances;
};

struct Bus {
	std::string name;
	std::vector<const Stop*> route;
	bool is_roundtrip = false;
};

using BusPtr = const Bus*;

enum class enStatRequestsType {
	STOP = 0,
	BUS,
	MAP,
	ROUTE,
	SERIALIZE,
};

struct StatRequest {
	int id = 0;
	enStatRequestsType type;
	std::string name;
	std::string from;
	std::string to;
};

using StopBuses = const std::set<BusPtr>*;

using AllBusesPtr = const std::unordered_map<std::string_view, BusPtr>*;