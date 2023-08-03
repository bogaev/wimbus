#pragma once

/*
 * В этом файле код, отвечающий за визуализацию карты маршрутов в формате SVG.
 */

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <array>
#include <string>
#include <variant>
#include <vector>
#include <algorithm>
#include <vector>
#include <iostream>
#include <deque>
#include <map>
#include <string_view>
#include <utility>

using MapStop = std::pair<std::string_view, svg::Point>;
using MapStops = std::vector<MapStop>;
struct RouteAttr {
    MapStops stops;
    svg::Color color{};
    bool is_roundtrip;
};
using MapBus = const std::pair<std::string_view, RouteAttr>;
using MapBuses = std::map<std::string_view, RouteAttr>;

struct RenderSettings {
    double width = 600.0;
    double height = 400.0;

    double padding = 50.0;

    double line_width = 14.0;
    double stop_radius = 5.0;

    int bus_label_font_size = 20;
    std::array<double, 2> bus_label_offset = { 7.0, 15.0 };

    int stop_label_font_size = 20;
    std::array<double, 2> stop_label_offset = { 7.0, -3.0 };

    svg::Color underlayer_color = { svg::Rgba{ 255, 255, 255, 0.85} };
    double underlayer_width = 3.0;

    std::vector<svg::Color> color_palette{ "green", svg::Rgb{255, 160, 0}, "red", "black"};

    svg::Color getColor() {
        svg::Color color = color_palette[color_counter % color_palette.size()];
        ++color_counter;
        return color;
    }

    void Clear() {
        width = 0.;
        height = 0.;
        padding = 0.;
        line_width = 0.;
        stop_radius = 0.;
        bus_label_font_size = 0;
        bus_label_offset = { 0., 0. };
        stop_label_font_size = 0;
        stop_label_offset = { 0., 0. };
        underlayer_color = {};
        underlayer_width = 0.;
        color_palette.clear();
    }

private:
    size_t color_counter = 0;
};

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
    using StopNamesToPoints = std::map<std::string_view, svg::Point>;
public:
    MapRenderer() = default;

    void SetRenderSettings(std::optional<RenderSettings> settings);
    template<typename Container>
    MapBuses CoordinatesToPoints(Container buses);

    std::vector<svg::Polyline> RenderRouteLines(MapBuses& buses);
    std::vector<svg::Text> RenderRouteNames(MapBuses& buses);
    void RenderStops(MapBuses& buses, std::vector<svg::Circle>* circles_out,
                                      std::vector<svg::Text>* names_out);

private:
    svg::Polyline RenderRoute(MapBus bus);
    std::vector<svg::Circle> DrawCircles(const StopNamesToPoints& stops);
    std::vector<svg::Text> DrawText(const StopNamesToPoints& stops);
    RenderSettings settings_{};
};

template<typename Container>
MapBuses MapRenderer::CoordinatesToPoints(Container buses) {
    std::vector<geo::Coordinates> all_stops_coord;
    all_stops_coord.reserve(1000);

    for (const auto& bus : *buses) {
        for (const auto& stop : bus.second->route) {
            all_stops_coord.push_back(stop->coord);
        }
    }
    SphereProjector projector(
        all_stops_coord.cbegin(), all_stops_coord.cend(),
        settings_.width, settings_.height, settings_.padding
    );

    MapBuses map_buses;
    for (const auto& bus : *buses) {
        if (bus.second->route.empty()) continue;
        RouteAttr route;
        for (const auto& stop : bus.second->route) {
            route.stops.push_back({ stop->name, projector(stop->coord) });
            route.is_roundtrip = bus.second->is_roundtrip;
        }
        map_buses[bus.first] = std::move(route);
    }

    return map_buses;
}