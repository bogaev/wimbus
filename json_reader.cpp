#include "transport_router.h"
#include "json_reader.h"

using namespace std;
using namespace in;

JsonReader::JsonReader(RequestHandler& req_handler)
	: req_handler_(req_handler)
{}

void JsonReader::ReadDocument(istream& input) {
	auto query_ = json::Load(input);
	if (!query_.GetRoot().IsDict()) {
		throw invalid_argument("Invalid document"s);
	}
	auto all_reqs = query_.GetRoot().AsDict();

	if (all_reqs.count(BASE_REQS)) {
		if (!(all_reqs[BASE_REQS].IsArray())) {
			throw invalid_argument("Invalid document: Base requests wrong format"s);
		}
		ReadStop(all_reqs[BASE_REQS].AsArray());
		ReadBus(all_reqs[BASE_REQS].AsArray());
	}
	if (all_reqs.count(STAT_REQS)) {
		if (!(all_reqs[STAT_REQS].IsArray())) {
			throw invalid_argument("Invalid document: Stat requests wrong format"s);
		}
		ReadStatReqs(all_reqs[STAT_REQS].AsArray());
	}
	if (all_reqs.count(RENDER_SETTINGS)) {
		if (!(all_reqs[RENDER_SETTINGS].IsDict())) {
			throw invalid_argument("Invalid document: Render settings wrong format"s);
		}
		ReadRenderSettings(all_reqs[RENDER_SETTINGS].AsDict());
	}
	if (all_reqs.count(ROUTINGS_SETTINGS)) {
		if (!(all_reqs[ROUTINGS_SETTINGS].IsDict())) {
			throw invalid_argument("Invalid document: Render settings wrong format"s);
		}
		ReadRoutingSettings(all_reqs[ROUTINGS_SETTINGS].AsDict());
	}
	if (all_reqs.count(SERIALIZATION_SETTINGS)) {
		if (!(all_reqs[SERIALIZATION_SETTINGS].IsDict())) {
			throw invalid_argument("Invalid document: Render settings wrong format"s);
		}
		ReadSerializationSettings(all_reqs[SERIALIZATION_SETTINGS].AsDict());
	}
}

void JsonReader::ReadSerializationSettings(Dict request) {
	string filename;
	if (request.count("file"s)) {
		filename = request.at("file"s).AsString();
	}
	req_handler_.AddSerializeSettings(filename);
}

void JsonReader::ReadRoutingSettings(Dict json) const  {
	router::RoutingSettings settings;
	for (auto s : json) {
		if (s.first == "bus_wait_time"s) {
			settings.bus_wait_time = s.second.AsInt();
		}
		else if (s.first == "bus_velocity"s) {
			settings.bus_velocity = s.second.AsDouble();
		}
		req_handler_.AddRoutingSettings(move(settings));
	}
}

void JsonReader::ReadRenderSettings(Dict settings) const {
	RenderSettings draw_settings;
	for (auto s : settings) {
		if (s.first == "width"s) {
			draw_settings.width = s.second.AsDouble();
		}
		else if (s.first == "height"s) {
			draw_settings.height = s.second.AsDouble();
		}
		else if (s.first == "padding"s) {
			draw_settings.padding = s.second.AsDouble();
		}
		else if (s.first == "line_width"s) {
			draw_settings.line_width = s.second.AsDouble();
		}
		else if (s.first == "stop_radius"s) {
			draw_settings.stop_radius = s.second.AsDouble();
		}
		else if (s.first == "bus_label_font_size"s) {
			draw_settings.bus_label_font_size = s.second.AsInt();
		}
		else if (s.first == "bus_label_offset"s) {
			if (s.second.IsArray()) {
				auto arr = s.second.AsArray();
				for (size_t i = 0; i < arr.size(); ++i) {
					draw_settings.bus_label_offset[i] = arr[i].AsDouble();
				}
			}
		}
		else if (s.first == "stop_label_font_size"s) {
			draw_settings.stop_label_font_size = s.second.AsInt();
		}
		else if (s.first == "stop_label_offset"s) {
			if (s.second.IsArray()) {
				auto arr = s.second.AsArray();
				for (size_t i = 0; i < arr.size(); ++i) {
					draw_settings.stop_label_offset[i] = arr[i].AsDouble();
				}
			}
		}
		else if (s.first == "underlayer_color"s) {
			if (s.second.IsString()) {
				draw_settings.underlayer_color = s.second.AsString();
			}
			else if (s.second.IsArray()) {
				auto rgb = s.second.AsArray();
				if (rgb.size() == 3) {
					draw_settings.underlayer_color = svg::Rgb{ (uint8_t)rgb[0].AsInt(), (uint8_t)rgb[1].AsInt(), (uint8_t)rgb[2].AsInt() };
				}
				if (rgb.size() == 4) {
					draw_settings.underlayer_color = svg::Rgba{ (uint8_t)rgb[0].AsInt(), (uint8_t)rgb[1].AsInt(), (uint8_t)rgb[2].AsInt(), rgb[3].AsDouble() };
				}
			}
		}
		else if (s.first == "underlayer_width"s) {
			draw_settings.underlayer_width = s.second.AsDouble();
		}
		else if (s.first == "color_palette"s) {
			draw_settings.color_palette.clear();
			auto palette = s.second.AsArray();
			for (auto color : palette) {
				if (color.IsString()) {
					draw_settings.color_palette.push_back(color.AsString());
				}
				if (color.IsArray()) {
					auto rgb = color.AsArray();
					if (rgb.size() == 3) {
						draw_settings.color_palette.push_back(svg::Rgb{ (uint8_t)rgb[0].AsInt(), (uint8_t)rgb[1].AsInt(), (uint8_t)rgb[2].AsInt() });
					}
					if (rgb.size() == 4) {
						draw_settings.color_palette.push_back(svg::Rgba{ (uint8_t)rgb[0].AsInt(), (uint8_t)rgb[1].AsInt(), (uint8_t)rgb[2].AsInt(), rgb[3].AsDouble() });
					}
				}
			}
		}
	}
	req_handler_.AddRenderSettings(draw_settings);
}

void JsonReader::PrintStatsRequests(ostream& output) const {
	Array response{};
	for (auto res : req_handler_.GetResponses()) {
		response.push_back(visit(StatsPrinter{ output, res.id }, move(res.stat)));
	}
	Print(Document{ response }, output);
}

void JsonReader::ReadStop(Array base_reqs) {
	for (auto req : base_reqs) {
		if (!req.IsDict()) {
			throw invalid_argument("Invalid input document: Request wrong format"s);
		}
		if (req.AsDict().at("type"s) == "Stop"s) {
			string stop_name;
			if (req.AsDict().count("name"s)) {
				stop_name = req.AsDict().at("name"s).AsString();
			}

			double latitude = 0;
			if (req.AsDict().count("latitude"s)) {
				latitude = req.AsDict().at("latitude"s).AsDouble();
			}

			double longitude = 0;
			if (req.AsDict().count("longitude"s)) {
				longitude = req.AsDict().at("longitude"s).AsDouble();
			}

			std::unordered_map<std::string, int> road_distances;
			Dict distancies;
			if (req.AsDict().count("road_distances"s)) {
				distancies = req.AsDict().at("road_distances"s).AsDict();
			}

			for (auto d : distancies) {
				road_distances[d.first] = d.second.AsInt();
			}

			req_handler_.AddStopRequest({ move(stop_name), move(geo::Coordinates{latitude, longitude}), move(road_distances) });
		}
	}
}

void JsonReader::ReadBus(Array base_reqs) {
	BusRoute route;
	for (auto req : base_reqs) {
		if (!req.IsDict()) {
			throw invalid_argument("Invalid input document: Request wrong format"s);
		}
		if (req.AsDict().at("type"s) == "Bus"s) {
			if (req.AsDict().count("name"s)) {
				route.name = req.AsDict().at("name"s).AsString();
			}

			Array stops;
			if (req.AsDict().count("stops"s)) {
				stops = req.AsDict().at("stops"s).AsArray();
			}

			for (auto stop : stops) {
				route.stops.push_back(stop.AsString());
			}

			if (req.AsDict().count("is_roundtrip"s)) {
				route.is_roundtrip = req.AsDict().at("is_roundtrip"s).AsBool();
			}

			req_handler_.AddBusRequest(move(route));
		}
	}
}

void JsonReader::ReadStatReqs(Array stat_reqs) {
	for (auto req : stat_reqs) {
		int id = 0;
		if (req.AsDict().count("id"s)) {
			id = req.AsDict().at("id"s).AsInt();
		}

		string type;
		if (req.AsDict().count("type"s)) {
			type = req.AsDict().at("type"s).AsString();
		}
		if (type == "Map"s) {
			req_handler_.AddStatRequest({ id, enStatRequestsType::MAP, {} });
			continue;
		}

		if (type == "Route"s) {
			if (req.AsDict().count("from"s)) {
				string from = req.AsDict().at("from"s).AsString();
				string to = req.AsDict().at("to"s).AsString();
				req_handler_.AddStatRequest(
					{ id, enStatRequestsType::ROUTE, {}, from, to }
				);
			}
			continue;
		}

		string name;
		if (req.AsDict().count("name"s)) {
			name = req.AsDict().at("name"s).AsString();
		}

		auto req_type = (type == "Stop"s ? enStatRequestsType::STOP : enStatRequestsType::BUS);
		req_handler_.AddStatRequest({ id, req_type, name });

	}
}