#include "serialization.h"

using namespace std;

namespace ptb {

void Protobuffer::SerializeDB(std::string filename) {
	pbf_db::TransportCatalogue base;
	ofstream output(filename, ios::binary);
	const deque<Stop>& stops = db_.stops_;
	const deque<Bus>& buses = db_.buses_;
	{
		for (const Stop& stop : stops) {
			pbf_db::Stop stop_pb;
			uint64_t id_s = stop_name_to_id_.at(stop.name);
			stop_pb.set_id(id_s);
			stop_pb.set_name(stop.name);

			pbf_db::Coordinates coord_out;
			coord_out.set_lat(stop.coord.lat);
			coord_out.set_lng(stop.coord.lng);
			*stop_pb.mutable_coord() = coord_out;

			for (const auto& distance : stop.road_distances) {
				uint64_t id_d = stop_name_to_id_.at(distance.first);
				(*stop_pb.mutable_road_distances())[id_d] = distance.second;
			}
			*base.add_stop() = move(stop_pb);
		}
	}
	{
		for (const Bus& bus : buses) {
			pbf_db::Bus bus_pb;
			bus_pb.set_name(bus.name);
			bus_pb.set_is_roundtrip(bus.is_roundtrip);
			for (const auto& stop : bus.route) {
				uint64_t id_d = stop_name_to_id_.at(stop->name);
				bus_pb.add_route(id_d);
			}
			*base.add_bus() = move(bus_pb);
		}
	}

	if (render_settings_) {
		*base.mutable_render_settings() = move(SerializeRenderSettings());
	}

	if (router_settings_) {
		*base.mutable_graph() = move(SerializeGraph());
		*base.mutable_router() = move(SerializeRouter());
	}

	base.SerializePartialToOstream(&output);
}

void Protobuffer::DeserializeDB(std::string filename) {
	pbf_db::TransportCatalogue base;
	map<string, const Stop*> name_to_stop_ptr_;
	std::ifstream input(filename, ios::binary);

	db_.stops_.clear();
	db_.buses_.clear();
	db_.stopname_to_stop_.clear();
	db_.busname_to_bus_.clear();
	db_.stopname_to_buses_.clear();
	db_.stops_distance_.clear();

	if (!base.ParseFromIstream(&input)) {
		return;
	}

	for (const auto& stop : base.stop()) {
		uint64_t id = stop.id();
		string name = stop.name();
		id_to_stop_name_[id] = name;
	}

	for (const auto& stop : base.stop()) {
		Stop stop_;
		stop_.name = stop.name();
		stop_.coord.lat = stop.coord().lat();
		stop_.coord.lng = stop.coord().lng();

		for (const auto& distance : stop.road_distances()) {
			string name = id_to_stop_name_.at(distance.first);
			stop_.road_distances[name] = distance.second;
		}
		auto& last_added = db_.stops_.emplace_back(stop_);
		name_to_stop_ptr_[stop_.name] = &last_added;

		db_.stopname_to_stop_.insert({ last_added.name, &last_added });
		db_.stopname_to_buses_[last_added.name];
	}

	for (auto& stop : db_.stops_) {
		for (auto& distance : stop.road_distances) {
			db_.SetStopsDistance(stop.name, distance.first, distance.second);
		}
	}

	for (const auto& bus : base.bus()) {
		Bus bus_;
		bus_.name = bus.name();
		bus_.is_roundtrip = bus.is_roundtrip();

		for (const auto& stop : bus.route()) {
			string name = id_to_stop_name_.at(stop);
			bus_.route.push_back(name_to_stop_ptr_.at(name));
		}
		auto& last_added = db_.buses_.emplace_back(bus_);

		db_.busname_to_bus_.insert({ last_added.name, &last_added });
		for (auto stop : bus_.route) {
			db_.stopname_to_buses_[stop->name].insert(&last_added);
		}
	}

	if (base.has_render_settings()) {
		DeserializeRenderSettings(move(*base.mutable_render_settings()));
	}

	if (base.has_graph()) {
		DeserializeGraph(move(*base.mutable_graph()));
	}

	if (base.has_router()) {
		DeserializeRouter(move(*base.mutable_router()));
	}
}

// private section
pbf_db::RenderSettings Protobuffer::SerializeRenderSettings() {
	pbf_db::RenderSettings out;

	if (render_settings_ == nullptr) {
		return out;
	}

	const RenderSettings& in = *render_settings_;

	out.set_width(in.width);
	out.set_height(in.height);

	out.set_padding(in.padding);

	out.set_line_width(in.line_width);
	out.set_stop_radius(in.stop_radius);

	out.set_bus_label_font_size(in.bus_label_font_size);
	out.add_bus_label_offset(in.bus_label_offset[0]);
	out.add_bus_label_offset(in.bus_label_offset[1]);

	out.set_stop_label_font_size(in.stop_label_font_size);
	out.add_stop_label_offset(in.stop_label_offset[0]);
	out.add_stop_label_offset(in.stop_label_offset[1]);

	{
		pbf_db::Color out_color;
		visit(ColorSerializePrinter{ out_color }, in.underlayer_color);
		*out.mutable_underlayer_color() = move(out_color);
	}

	out.set_underlayer_width(in.underlayer_width);

	for (const auto& c : in.color_palette) {
		pbf_db::Color out_color;
		visit(ColorSerializePrinter{ out_color }, c);
		*out.add_color_palette() = out_color;
	}

	return out;
}

void Protobuffer::DeserializeRenderSettings(pbf_db::RenderSettings in) {
	if (render_settings_ == nullptr) {
		return;
	}

	RenderSettings& out = *render_settings_;

	out.Clear();

	out.width = in.width();
	out.height = in.height();

	out.padding = in.padding();

	out.line_width = in.line_width();
	out.stop_radius = in.stop_radius();

	out.bus_label_font_size = in.bus_label_font_size();
	for (int i = 0; i < in.bus_label_offset_size(); ++i) {
		out.bus_label_offset[i] = in.bus_label_offset(i);
	}

	out.stop_label_font_size = in.stop_label_font_size();
	for (int i = 0; i < in.stop_label_offset_size(); ++i) {
		out.stop_label_offset[i] = in.stop_label_offset(i);
	}

	{
		variant<pbf_db::ColorString, pbf_db::ColorRBG> pbf_color;
		if (in.underlayer_color().has_rgb()) {
			pbf_color = in.underlayer_color().rgb();
		}
		else if (in.underlayer_color().has_str()) {
			pbf_color = in.underlayer_color().str();
		}
		svg::Color db_color;
		visit(ColorDeserializePrinter{ db_color }, pbf_color);
		out.underlayer_color = db_color;
	}
	out.underlayer_width = in.underlayer_width();

	for (const auto& c : in.color_palette()) {
		variant<pbf_db::ColorString, pbf_db::ColorRBG> pbf_color;
		if (c.has_rgb()) {
			pbf_color = c.rgb();
		}
		else if (c.has_str()) {
			pbf_color = c.str();
		}
		svg::Color db_color;
		visit(ColorDeserializePrinter{ db_color }, move(pbf_color));
		out.color_palette.push_back(move(db_color));
	}
}

pbf_db::Graph Protobuffer::SerializeGraph() {
	pbf_db::Graph out;

	const auto& in = router_->graph_;

	for (const auto& e : in->edges_) {
		pbf_db::Edge edge_pbf;
		edge_pbf.set_from(e.from);
		edge_pbf.set_to(e.to);
		edge_pbf.set_weight(e.weight);
		*out.add_edges() = move(edge_pbf);
	}

	for (const auto& lists : in->incidence_lists_) {
		pbf_db::IncidenceList list_pbf;
		for (auto edge_id : lists) {
			list_pbf.add_edge_id(edge_id);
		}
		*out.add_incidence_lists() = move(list_pbf);
	}

	return out;
}

void Protobuffer::DeserializeGraph(pbf_db::Graph in) {
	auto& out = router_->graph_;

	for (const auto& edge_pbf : in.edges()) {
		graph::Edge<double> edge;
		edge.from = edge_pbf.from();
		edge.to = edge_pbf.to();
		edge.weight = edge_pbf.weight();
		out->edges_.push_back(move(edge));
	}

	for (const auto& list_pbf : in.incidence_lists()) {
		std::vector<size_t> list;
		for (auto& edge_pbf : list_pbf.edge_id()) {
			list.push_back(move(edge_pbf));
		}
		out->incidence_lists_.push_back(move(list));
	}
}

pbf_db::Router Protobuffer::SerializeRouter() {
	pbf_db::Router data;

	{ // router settings
		const auto& in = router_->settings_;
		data.mutable_settings()->set_bus_wait_time(in.bus_wait_time);
		data.mutable_settings()->set_bus_velocity(in.bus_velocity);
	}
	{ // router internal data
		const auto& in = router_->router_;
		for (const auto& edges : in->routes_internal_data_) {
			pbf_db::EdgeInternalData edges_pbf;
			for (const auto& edge : edges) {
				pbf_db::OptionalRouteInternalData optional_edge_pbf;
				if (edge.has_value()) {
					pbf_db::RouteInternalData edge_pbf;
					edge_pbf.set_weight(edge.value().weight);
					if (edge.value().prev_edge.has_value()) {
						pbf_db::PrevEdge prev_edge_pbf;
						prev_edge_pbf.set_value(edge.value().prev_edge.value());
						*edge_pbf.mutable_prev_edge() = move(prev_edge_pbf);
					}
					*optional_edge_pbf.mutable_route_internal_data() = move(edge_pbf);
				}
				*edges_pbf.add_edge() = move(optional_edge_pbf);
			}
			*data.add_edges() = move(edges_pbf);
		}
	}
	{ // stop to vertex id index
		const auto& in = router_->stop_to_vertex_id_;
		for (auto& stop_to_vertex : in) {
			uint64_t id = stop_name_to_id_.at(stop_to_vertex.first->name);
			pbf_db::VertexPair vertex_pair_pbf;
			vertex_pair_pbf.set_vertex1(move(stop_to_vertex.second.first));
			vertex_pair_pbf.set_vertex2(move(stop_to_vertex.second.second));
			(*data.mutable_stop_to_vertex_id())[id] = move(vertex_pair_pbf);
		}
	}
	{ // edge id to item index
		const auto& in = router_->edge_id_to_item_;
		for (auto& edge_id_to_item : in) {
			auto edge_id = edge_id_to_item.first;
			pbf_db::RouteItem item_pbf;
			visit(RouteItemVisiter{ item_pbf }, move(edge_id_to_item.second));
			(*data.mutable_edge_id_to_item())[edge_id] = move(item_pbf);
		}
	}

	return data;
}

void Protobuffer::DeserializeRouter(pbf_db::Router table) {
	{
		auto& out = router_->settings_;
		out.bus_wait_time = table.settings().bus_wait_time();
		out.bus_velocity = table.settings().bus_velocity();
		*router_settings_ = router_->settings_;
	}
	{
		auto& out = *router_->router_;
		for (const auto& edges_pbf : table.edges()) {
			vector<optional<graph::Router<double>::RouteInternalData>> edges;
			for (const auto& edge_pbf : edges_pbf.edge()) {
				optional<graph::Router<double>::RouteInternalData> edge;
				if (edge_pbf.has_route_internal_data()) {
					graph::Router<double>::RouteInternalData tmp;
					tmp.weight = edge_pbf.route_internal_data().weight();
					if (edge_pbf.route_internal_data().has_prev_edge()) {
						tmp.prev_edge = edge_pbf.route_internal_data().prev_edge().value();
					}
					edge = move(tmp);
				}
				edges.push_back(move(edge));
			}
			out.routes_internal_data_.push_back(move(edges));
		}
	}
	{ // stop to vertex id index
		auto& out = router_->stop_to_vertex_id_;
		for (auto& stop_to_vertex_pbf : table.stop_to_vertex_id()) {
			string stop_name = id_to_stop_name_.at(stop_to_vertex_pbf.first);
			const Stop* stop = db_.stopname_to_stop_.at(stop_name);
			std::pair<graph::VertexId, graph::VertexId> vertex_pair;
			vertex_pair.first = move(stop_to_vertex_pbf.second.vertex1());
			vertex_pair.second = move(stop_to_vertex_pbf.second.vertex2());
			out[stop] = move(vertex_pair);
		}
	}
	{ // edge id to item index
		auto& out = router_->edge_id_to_item_;
		for (auto& edge_id_to_item_pbf : table.edge_id_to_item()) {
			size_t edge_id = edge_id_to_item_pbf.first;
			if (edge_id_to_item_pbf.second.has_bus()) {
				router::BusItem bus;
				bus.bus_name = move(edge_id_to_item_pbf.second.bus().bus_name());
				bus.span_count = move(edge_id_to_item_pbf.second.bus().span_count());
				bus.time = move(edge_id_to_item_pbf.second.bus().time());
				out[edge_id] = move(bus);
			}
			else if (edge_id_to_item_pbf.second.has_wait()) {
				router::WaitItem wait;
				wait.stop_name = move(edge_id_to_item_pbf.second.wait().stop_name());
				wait.time = move(edge_id_to_item_pbf.second.wait().time());
				out[edge_id] = move(wait);
			}
		}
	}
}

} // ptb