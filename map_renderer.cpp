#include "map_renderer.h"

using namespace std;
using namespace svg;

void MapRenderer::SetRenderSettings(optional<RenderSettings> settings) {
    if (settings.has_value()) {
        settings_ = settings.value();
    }
}

vector<Polyline> MapRenderer::RenderRouteLines(MapBuses& buses) {
    vector<Polyline> route_lines;
    for (auto& bus : buses) {
        if (bus.second.stops.empty()) {
            continue;
        }
        auto lines = RenderRoute(bus);
        lines.SetFillColor({})
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        bus.second.color = settings_.getColor();
        lines.SetStrokeColor(bus.second.color);
        route_lines.push_back(std::move(lines));
    }
    return route_lines;
}

vector<Text> MapRenderer::RenderRouteNames(MapBuses& buses) {
    vector<Text> text;
    for (auto& bus : buses) {
        if (bus.second.stops.empty()) {
            continue;
        }
        auto first_stop = *(bus.second.stops.begin());
        auto last_stop = *(bus.second.stops.rbegin());
        Text stop_underlayer;
        Text stop_name;
        stop_underlayer.SetPosition(first_stop.second)
            .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
            .SetFontSize((uint32_t)settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(string(bus.first))
            .SetStrokeColor(settings_.underlayer_color)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        stop_name.SetPosition(first_stop.second)
            .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
            .SetFontSize((uint32_t)settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(string(bus.first))
            .SetFillColor(bus.second.color);
        text.push_back(stop_underlayer);
        text.push_back(stop_name);

        if (!bus.second.is_roundtrip && 
            string(first_stop.first) != string(last_stop.first)) {
            stop_underlayer.SetPosition(last_stop.second);
            stop_name.SetPosition(last_stop.second);
            text.push_back(stop_underlayer);
            text.push_back(stop_name);
        }
    }
    return text;
}

Polyline MapRenderer::RenderRoute(MapBus bus) {
    Polyline polyline;
    for (auto stop : bus.second.stops) {
        polyline.AddPoint(move(stop.second));
    }
    if (!bus.second.is_roundtrip) {
        auto stops = bus.second.stops;
        for (auto stop = next(stops.rbegin()); stop != stops.rend(); ++stop) {
            polyline.AddPoint(move(stop->second));
        }
    }
    return polyline;
}

void MapRenderer::RenderStops(MapBuses& buses,
                                    std::vector<svg::Circle>* circles_out,
                                    std::vector<svg::Text>* names_out) {
    StopNamesToPoints stop_to_point;

    for (const auto& bus : buses) {
        for (const auto& stop : bus.second.stops) {
            stop_to_point[stop.first] = stop.second;
        }
    }

    *circles_out = move(DrawCircles(stop_to_point));
    *names_out = move(DrawText(stop_to_point));
}

vector<Circle> MapRenderer::DrawCircles(const StopNamesToPoints& stops) {
    vector<Circle> circles;
    circles.reserve(1000);

    for (const auto& stop : stops) {
        Point center = stop.second;
        Circle circle;
        circles.push_back(circle
            .SetCenter(center)
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"));
    }
    return circles;
}

vector<Text> MapRenderer::DrawText(const StopNamesToPoints& stops) {
    vector<Text> stop_names;
    stop_names.reserve(1000);

    for (const auto& stop : stops) {
        Point center = stop.second;
        Text stop_underlayer;
        Text stop_name;
        stop_names.push_back(stop_underlayer
            .SetPosition(center)
            .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetData(string(stop.first)));
        stop_names.push_back(stop_name
            .SetPosition(center)
            .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor("black")
            .SetData(string(stop.first)));
    }
    return stop_names;
}