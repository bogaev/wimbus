#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "graph.h"
#include "router.h"

#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>
#include <graph.pb.h>

#include <string>
#include <fstream>
#include <deque>
#include <variant>
#include <optional>
#include <map>

namespace ptb {

struct Settings {
	std::string filename;
};

class Protobuffer {
public:
    Protobuffer(transport_db::TransportCatalogue& db)
        : db_(db)
    {
        StopsIdInit();
    }

    Protobuffer(transport_db::TransportCatalogue& db,
        RenderSettings* render_settings,
        router::RoutingSettings* router_settings,
        router::TransportRouter* router)
        : db_(db)
        , render_settings_(render_settings)
        , router_settings_(router_settings)
        , router_(router)
    {
        StopsIdInit();
    }

    void SerializeDB(std::string filename);
    void DeserializeDB(std::string filename);

private:
	transport_db::TransportCatalogue& db_;

    RenderSettings* render_settings_;
    router::RoutingSettings* router_settings_;
    router::TransportRouter* router_;

    std::map<std::string, uint64_t> stop_name_to_id_;
    std::map<uint64_t, std::string> id_to_stop_name_;

    pbf_db::RenderSettings SerializeRenderSettings();
    void DeserializeRenderSettings(pbf_db::RenderSettings in);

    pbf_db::Graph SerializeGraph();
    void DeserializeGraph(pbf_db::Graph in);

    pbf_db::Router SerializeRouter();
    void DeserializeRouter(pbf_db::Router table);

    void StopsIdInit() {
        const std::deque<Stop>& stops = db_.stops_;
        uint64_t i = 0;
        for (const Stop& stop : stops) {
            stop_name_to_id_[stop.name] = i;
            ++i;
        }
    }
};

struct ColorSerializePrinter {
    pbf_db::Color& out_color;

    void operator()(std::monostate) const {
    }
    void operator()(const std::string& color) {
        out_color.mutable_str()->set_str(color);
    }
    void operator()(const svg::Rgb& rgb) {
        out_color.mutable_rgb()->set_red(rgb.red);
        out_color.mutable_rgb()->set_green(rgb.green);
        out_color.mutable_rgb()->set_blue(rgb.blue);
        out_color.mutable_rgb()->set_alpha(-1);
    }
    void operator()(const svg::Rgba& rgba) {
        out_color.mutable_rgb()->set_red(rgba.red);
        out_color.mutable_rgb()->set_green(rgba.green);
        out_color.mutable_rgb()->set_blue(rgba.blue);
        out_color.mutable_rgb()->set_alpha(rgba.opacity);
    }
};

struct ColorDeserializePrinter {
    svg::Color& db_color;

    void operator()(std::monostate) const {
    }
    void operator()(const pbf_db::ColorString& string) {
        db_color = string.str();
    }
    void operator()(const pbf_db::ColorRBG& rgb) {
        if (rgb.alpha() < 0) {
            svg::Rgb db_rgb;
            db_rgb.red = rgb.red();
            db_rgb.green = rgb.green();
            db_rgb.blue = rgb.blue();
            db_color = db_rgb;
        }
        else {
            svg::Rgba db_rgb;
            db_rgb.red = rgb.red();
            db_rgb.green = rgb.green();
            db_rgb.blue = rgb.blue();
            db_rgb.opacity = rgb.alpha();
            db_color = db_rgb;
        }
    }
};

struct RouteItemVisiter {
    pbf_db::RouteItem& item_pbf;

    void operator()(std::monostate) const {
    }
    void operator()(const router::WaitItem& wait) {
        pbf_db::WaitItem wait_pbf;
        wait_pbf.set_stop_name(std::string(wait.stop_name));
        wait_pbf.set_time(wait.time);
        *item_pbf.mutable_wait() = wait_pbf;
    }
    void operator()(const router::BusItem& bus) {
        pbf_db::BusItem bus_pbf;
        bus_pbf.set_bus_name(std::string(bus.bus_name));
        bus_pbf.set_span_count(bus.span_count);
        bus_pbf.set_time(bus.time);
        *item_pbf.mutable_bus() = bus_pbf;
    }
};

}