#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "svg.h"
#include "serialization.h"

#include <optional>
#include <memory>

/*
* Здесь код обработчика запросов к базе, содержащего логику, которую не
* хотелось бы помещать ни в transport_db, ни в json reader.
*
*/

namespace in {

struct BusRoute {
    std::string name{};
    std::vector<std::string> stops{};
    bool is_roundtrip = false;
};

struct Response {
    int id = 0;
    std::variant<
        std::monostate,
        std::optional<RouteStats>,
        std::optional<StopBuses>,
        std::shared_ptr<svg::Document>,
        std::optional<router::OptimalRoute>> stat;
};

class RequestHandler {
public:
    RequestHandler(transport_db::TransportCatalogue& db);
    RequestHandler(transport_db::TransportCatalogue& db,
        MapRenderer* map_renderer);

    // передаётся по значению, чтобы использовать семантику перемещения
    void AddStopRequest(Stop stop);
    // передаётся по значению, чтобы использовать семантику перемещения
    void AddBusRequest(BusRoute bus);
    // передаётся по значению, чтобы использовать семантику перемещения
    void AddStatRequest(StatRequest req);
    // передаётся по значению, чтобы использовать семантику перемещения
    void AddRenderSettings(RenderSettings settings);
    // передаётся по значению, чтобы использовать семантику перемещения
    void AddRoutingSettings(router::RoutingSettings settings);
    // передаётся по значению, чтобы использовать семантику перемещения
    void AddSerializeSettings(std::string settings);
    void ProcessBaseCreateRequests();
    void ProcessStatRequests();
    void SerializeBase();
    void DeserializeBase();
    const std::vector<Response>& GetResponses();

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<RouteStats> GetBusStat(const std::string_view& bus_name) const;
    AllBusesPtr GetAllBuses() const;

    // Возвращает маршруты, проходящие через
    std::optional<StopBuses> GetBusesByStop(const std::string_view& stop_name) const;

    template<typename Container>
    svg::Document RenderMap(Container buses) const;

private:
    void ProcessStopRequests();
    void ProcessBusRequests();

    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport_db::TransportCatalogue& db_;

    std::vector<Stop> stops_requests_;
    std::vector<BusRoute> buses_requests_;
    std::vector<StatRequest> stat_requests_;
    std::vector<Response> responses_;

    std::optional<ptb::Settings> serialize_settings_;
    std::optional<RenderSettings> render_settings_;
    std::unique_ptr<MapRenderer> map_renderer_;
    std::optional<router::RoutingSettings> routing_settings_;
    // Возвращает информацию об оптимальном маршруте для двух произвольных остановок
    std::unique_ptr<router::TransportRouter> router_;
};

template<typename Container>
svg::Document RequestHandler::RenderMap(Container buses) const {
    svg::Document doc;
    map_renderer_->SetRenderSettings(render_settings_);
    MapBuses map_buses = map_renderer_->CoordinatesToPoints(buses);

    // draw lines
    auto pathes = map_renderer_->RenderRouteLines(map_buses);
    for(auto lines : pathes)
        doc.Add(std::move(lines));

    // draw bus names
    auto names = map_renderer_->RenderRouteNames(map_buses);
    for (auto name : names)
        doc.Add(std::move(name));

    // draw stops
    std::vector<svg::Circle> circles;
    std::vector<svg::Text> stop_names;
    map_renderer_->RenderStops(map_buses, &circles, &stop_names);
    for (auto circle : circles)
        doc.Add(std::move(circle));
    for (auto stop_name : stop_names)
        doc.Add(std::move(stop_name));

    return doc;
}

} // namespace in
