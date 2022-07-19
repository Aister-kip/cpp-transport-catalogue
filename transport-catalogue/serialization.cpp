#include "serialization.h"

namespace proto_serialize {
	void Serialize::CatalogueSerialize() {
		tc_serialize::Catalogue proto_db;
		for (const auto& stop : *tc_ptr_->GetStopsDeque()) {
			tc_serialize::Stop proto_stop;
			proto_stop.set_name(stop.name);
			proto_stop.mutable_coordinates()->set_lat(stop.coordinates.lat);
			proto_stop.mutable_coordinates()->set_lng(stop.coordinates.lng);
			for (auto& [name, dist] : stop.stops_distance) {
				tc_serialize::StopsDistance proto_distance;
				proto_distance.set_id(tc_ptr_->GetStopId(name));
				proto_distance.set_distance_between(dist);
				*proto_stop.add_distance() = proto_distance;
			}
			*proto_db.add_stops() = proto_stop;
		}
		for (const auto& bus : *tc_ptr_->GetBusesDeque()) {
			tc_serialize::Bus proto_bus;
			proto_bus.set_name(bus.name);
			proto_bus.set_is_circular(bus.is_circular);
			for (const auto stop_ptr : bus.stops) {
				proto_bus.add_stops_id(tc_ptr_->GetStopId(stop_ptr->name));
			}
			*proto_db.add_buses() = proto_bus;
		}
		*proto_catalogue_.mutable_catalogue() = proto_db;
	}

	void Serialize::RendererSerialize() {
		const renderer::RenderSettings& settings = mp_ptr_->GetRenderSettings();
		tc_serialize::RenderSettings proto_settings;
		proto_settings.set_width(settings.width);
		proto_settings.set_height(settings.height);
		proto_settings.set_padding(settings.padding);
		proto_settings.set_line_width(settings.line_width);
		proto_settings.set_stop_radius(settings.stop_radius);
		proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
		proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
		proto_settings.set_underlayer_width(settings.underlayer_width);
		proto_settings.mutable_bus_label_offset()->set_x(settings.bus_label_offset.x);
		proto_settings.mutable_bus_label_offset()->set_y(settings.bus_label_offset.y);
		proto_settings.mutable_stop_label_offset()->set_x(settings.stop_label_offset.x);
		proto_settings.mutable_stop_label_offset()->set_y(settings.stop_label_offset.y);
		tc_serialize::Color underlayer_color;
		ProcessColor(underlayer_color, settings.underlayer_color);
		*proto_settings.mutable_undelayer_color() = underlayer_color;

		for (const auto& color : settings.color_palette) {
			tc_serialize::Color pallete_color;
			ProcessColor(pallete_color, color);
			*proto_settings.add_color_pallete() = pallete_color;
		}
		*proto_catalogue_.mutable_render_settings() = proto_settings;
	}

	void Serialize::ProcessColor(tc_serialize::Color& proto_color, const svg::Color& color) {
		if (std::holds_alternative<std::string>(color)) {
			proto_color.mutable_str()->set_color(std::get<std::string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb_color = std::get<svg::Rgb>(color);
			proto_color.mutable_rgb()->set_r(rgb_color.red);
			proto_color.mutable_rgb()->set_g(rgb_color.green);
			proto_color.mutable_rgb()->set_b(rgb_color.blue);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba_color = std::get<svg::Rgba>(color);
			proto_color.mutable_rgba()->mutable_rgb()->set_r(rgba_color.red);
			proto_color.mutable_rgba()->mutable_rgb()->set_g(rgba_color.green);
			proto_color.mutable_rgba()->mutable_rgb()->set_b(rgba_color.blue);
			proto_color.mutable_rgba()->set_opacity(rgba_color.opacity);
		}
	}

	void Serialize::RouterSerialize() {
		tc_serialize::TransportRouter proto_tr;
		proto_tr.mutable_route_settings()->set_bus_velocity(tr_ptr_->GetSettings().bus_velocity);
		proto_tr.mutable_route_settings()->set_bus_wait_time(tr_ptr_->GetSettings().bus_wait_time);
		for (const auto& [stop_ptr, id] : tr_ptr_->GetVertexes()) {
			tc_serialize::GraphVertex proto_vertex;
			proto_vertex.set_vertex_id(id);
			proto_vertex.set_stop_id(tc_ptr_->GetStopId(stop_ptr->name));
			*proto_tr.add_graph_vertexes() = proto_vertex;
		}
		for (const auto& edge : tr_ptr_->GetEdges()) {
			tc_serialize::TravelProps proto_edge;
			proto_edge.set_from_id(tc_ptr_->GetStopId(edge.from->name));
			proto_edge.set_to_id(tc_ptr_->GetStopId(edge.to->name));
			proto_edge.set_route(tc_ptr_->GetBusId(edge.route->name));
			proto_edge.mutable_travel_duration()->set_stops_number(edge.travel_duration.stops_number);
			proto_edge.mutable_travel_duration()->set_travel_time(edge.travel_duration.travel_time);
			proto_edge.mutable_travel_duration()->set_waiting_time(edge.travel_duration.waiting_time);
			*proto_tr.add_graph_edges() = proto_edge;
		}
		*proto_tr.mutable_graph() = ProcessGraph();
		*proto_tr.mutable_router() = ProcessRouter();

		*proto_catalogue_.mutable_transport_router() = proto_tr;
	}

	tc_serialize::Graph Serialize::ProcessGraph() {
		tc_serialize::Graph proto_graph;
		for (const auto& edge : tr_ptr_->GetGraph().GetEdges()) {
			tc_serialize::Edge proto_edge;
			proto_edge.set_from(edge.from);
			proto_edge.set_to(edge.to);
			proto_edge.mutable_weight()->set_stops_number(edge.weight.stops_number);
			proto_edge.mutable_weight()->set_waiting_time(edge.weight.waiting_time);
			proto_edge.mutable_weight()->set_travel_time(edge.weight.travel_time);
			*proto_graph.add_edges() = proto_edge;
		}
		for (const auto& lists : tr_ptr_->GetGraph().GetIncidentLists()) {
			tc_serialize::IncidenceList proto_list;
			for (const auto& list : lists) {
				proto_list.add_lists(list);
			}
			*proto_graph.add_incidence_lists() = proto_list;
		}
		return proto_graph;
	}

	tc_serialize::Router Serialize::ProcessRouter() {
		tc_serialize::Router proto_router;
		for (const auto& internal_data : tr_ptr_->GetRouter().GetInternalData()) {
			tc_serialize::RoutesInternalData proto_routes;
			for (const auto& data : internal_data) {
				tc_serialize::RouteInternalData proto_route;
				if (data.has_value()) {
					proto_route.mutable_rid_weight()->set_stops_number(data.value().weight.stops_number);
					proto_route.mutable_rid_weight()->set_travel_time(data.value().weight.travel_time);
					proto_route.mutable_rid_weight()->set_waiting_time(data.value().weight.waiting_time);
					if (data.value().prev_edge.has_value()) {
						proto_route.set_edge_prev(data.value().prev_edge.value());
					}
				}
				else {
					proto_route.set_empty(true);
				}
				*proto_routes.add_route_internal_data() = proto_route;
			}
			*proto_router.add_routes_internal_data() = proto_routes;
		}
		return proto_router;
	}

	void Serialize::SaveToFile(std::string& filename) {
		std::ofstream out(filename, std::ios::binary);
		CatalogueSerialize();
		RendererSerialize();
		RouterSerialize();

		proto_catalogue_.SerializeToOstream(&out);
	}

	void Deserialize::CatalogueDeserialize() {
		const tc_serialize::Catalogue& proto_db = proto_catalogue_.catalogue();
		for (size_t i = 0; i < proto_catalogue_.catalogue().stops_size(); ++i) {
			catalogue::Stop stop;
			stop.name = proto_db.stops(i).name();
			stop.coordinates.lat = proto_db.stops(i).coordinates().lat();
			stop.coordinates.lng = proto_db.stops(i).coordinates().lng();
			for (size_t j = 0; j < proto_db.stops(i).distance_size(); ++j) {
				stop.stops_distance[proto_db.stops(proto_db.stops(i).distance(j).id()).name()] = proto_db.stops(i).distance(j).distance_between();
			}
			tc_ptr_->AddStop(stop.name, stop.coordinates, stop.stops_distance);
		}
		tc_ptr_->SetDistances();

		for (size_t i = 0; i < proto_db.buses_size(); ++i) {
			std::string name = proto_db.buses(i).name();
			bool is_circular = proto_db.buses(i).is_circular();
			std::vector<std::string_view> bus_stops;
			for (size_t j = 0; j < proto_db.buses(i).stops_id_size(); ++j) {
				bus_stops.push_back(proto_db.stops(proto_db.buses(i).stops_id(j)).name());
			}
			tc_ptr_->AddRoute(name, bus_stops, is_circular);
		}
	}

	void Deserialize::RendererDeserialize() {
		renderer::RenderSettings settings;
		settings.width = proto_catalogue_.render_settings().width();
		settings.height = proto_catalogue_.render_settings().height();
		settings.padding = proto_catalogue_.render_settings().padding();
		settings.line_width = proto_catalogue_.render_settings().line_width();
		settings.stop_radius = proto_catalogue_.render_settings().stop_radius();
		settings.bus_label_font_size = proto_catalogue_.render_settings().bus_label_font_size();
		settings.stop_label_font_size = proto_catalogue_.render_settings().stop_label_font_size();
		settings.underlayer_width = proto_catalogue_.render_settings().underlayer_width();
		settings.bus_label_offset.x = proto_catalogue_.render_settings().bus_label_offset().x();
		settings.bus_label_offset.y = proto_catalogue_.render_settings().bus_label_offset().y();
		settings.stop_label_offset.x = proto_catalogue_.render_settings().stop_label_offset().x();
		settings.stop_label_offset.y = proto_catalogue_.render_settings().stop_label_offset().y();
		settings.underlayer_color = ProcessColor(proto_catalogue_.render_settings().undelayer_color());
		for (const auto& color : proto_catalogue_.render_settings().color_pallete()) {
			settings.color_palette.push_back(ProcessColor(color));
		}
		mp_ptr_->SetRenderSettings(settings);
	}

	svg::Color Deserialize::ProcessColor(const tc_serialize::Color& proto_color) const {
		if (proto_color.has_str()) {
			return proto_color.str().color();
		}
		else if (proto_color.has_rgb()) {
			svg::Rgb rgb;
			rgb.red = proto_color.rgb().r();
			rgb.green = proto_color.rgb().g();
			rgb.blue = proto_color.rgb().b();
			return rgb;
		}
		else if (proto_color.has_rgba()) {
			svg::Rgba rgba;
			rgba.red = proto_color.rgba().rgb().r();
			rgba.green = proto_color.rgba().rgb().g();
			rgba.blue = proto_color.rgba().rgb().b();
			rgba.opacity = proto_color.rgba().opacity();
			return rgba;
		}
	}

	void Deserialize::RouterDeserialize() {
		const tc_serialize::TransportRouter& proto_tr = proto_catalogue_.transport_router();
		domain::RouteSettings settings;
		settings.bus_velocity = proto_tr.route_settings().bus_velocity();
		settings.bus_wait_time = proto_tr.route_settings().bus_wait_time();
		std::unordered_map<domain::StopPtr, graph::VertexId> graph_vertexes;
		for (auto i = 0; i < proto_tr.graph_vertexes_size(); ++i) {
			graph_vertexes[&tc_ptr_->GetStopsDeque()->at(proto_tr.graph_vertexes(i).stop_id())] = proto_tr.graph_vertexes(i).vertex_id();
		}
		std::vector<transport_router::TravelProps> graph_edges;
		for (auto i = 0; i < proto_tr.graph_edges_size(); ++i) {
			transport_router::TravelProps travel_props;
			travel_props.from = &tc_ptr_->GetStopsDeque()->at(proto_tr.graph_edges(i).from_id());
			travel_props.to = &tc_ptr_->GetStopsDeque()->at(proto_tr.graph_edges(i).to_id());
			travel_props.route = &tc_ptr_->GetBusesDeque()->at(proto_tr.graph_edges(i).route());
			travel_props.travel_duration.stops_number = proto_tr.graph_edges(i).travel_duration().stops_number();
			travel_props.travel_duration.travel_time = proto_tr.graph_edges(i).travel_duration().travel_time();
			travel_props.travel_duration.waiting_time = proto_tr.graph_edges(i).travel_duration().waiting_time();
			graph_edges.push_back(std::move(travel_props));
		}
		auto graph_ptr = std::move(ProcessGraph());
		auto router_ptr = std::move(ProcessRouter(*graph_ptr));
		tr_ptr_->InitRouter(settings, tc_ptr_, std::move(graph_ptr), graph_vertexes, graph_edges, std::move(router_ptr));
	}

	std::unique_ptr<graph::DirectedWeightedGraph<transport_router::TravelDuration>> Deserialize::ProcessGraph() {
		using IncidenceList = std::vector<graph::EdgeId>;

		std::vector<IncidenceList> incidence_list;
		std::vector<graph::Edge<transport_router::TravelDuration>> edges;
		const tc_serialize::Graph& proto_graph = proto_catalogue_.transport_router().graph();
		for (auto i = 0; i < proto_graph.edges_size(); ++i) {
			graph::Edge<transport_router::TravelDuration> edge;
			edge.from = proto_graph.edges(i).from();
			edge.to = proto_graph.edges(i).to();
			edge.weight.stops_number = proto_graph.edges(i).weight().stops_number();
			edge.weight.travel_time = proto_graph.edges(i).weight().travel_time();
			edge.weight.waiting_time = proto_graph.edges(i).weight().waiting_time();
			edges.push_back(std::move(edge));
		}

		for (auto i = 0; i < proto_graph.incidence_lists_size(); ++i) {
			IncidenceList list;
			for (auto j = 0; j < proto_graph.incidence_lists(i).lists_size(); ++j) {
				list.push_back(proto_graph.incidence_lists(i).lists(j));
			}
			incidence_list.push_back(std::move(list));
		}
		std::unique_ptr<graph::DirectedWeightedGraph<transport_router::TravelDuration>> graph_ptr =
			std::make_unique<graph::DirectedWeightedGraph<transport_router::TravelDuration>>(std::move(edges), std::move(incidence_list));
		return graph_ptr;
	}

	std::unique_ptr<graph::Router<transport_router::TravelDuration>> Deserialize::ProcessRouter(graph::DirectedWeightedGraph<transport_router::TravelDuration>& graph) {
		const tc_serialize::Router& proto_router = proto_catalogue_.transport_router().router();

		graph::Router<transport_router::TravelDuration>::RoutesInternalData route_internal_data;
		for (auto i = 0; i < proto_router.routes_internal_data_size(); ++i) {
			std::vector<std::optional<graph::Router<transport_router::TravelDuration>::RouteInternalData>> vec_data;
			for (auto j = 0; j < proto_router.routes_internal_data(i).route_internal_data_size(); ++j) {
				graph::Router<transport_router::TravelDuration>::RouteInternalData data;
				if (proto_router.routes_internal_data(i).route_internal_data(j).empty() != true) {
					if (proto_router.routes_internal_data(i).route_internal_data(j).prev_edge_case()) {
						data.prev_edge = proto_router.routes_internal_data(i).route_internal_data(j).edge_prev();
					}
					data.weight.stops_number = proto_router.routes_internal_data(i).route_internal_data(j).rid_weight().stops_number();
					data.weight.travel_time = proto_router.routes_internal_data(i).route_internal_data(j).rid_weight().travel_time();
					data.weight.waiting_time = proto_router.routes_internal_data(i).route_internal_data(j).rid_weight().waiting_time();
					vec_data.push_back(std::move(data));
				}
				else {
					vec_data.push_back(std::nullopt);
				}
			}
			route_internal_data.push_back(std::move(vec_data));
		}
		std::unique_ptr<graph::Router<transport_router::TravelDuration>> router_ptr =
			std::make_unique<graph::Router<transport_router::TravelDuration>>(graph, route_internal_data);
		return router_ptr;
	}

	void Deserialize::ReadFromFile(std::string& filename) {
		std::ifstream in(filename, std::ios::binary);
		proto_catalogue_.ParseFromIstream(&in);
		CatalogueDeserialize();
		RendererDeserialize();
		RouterDeserialize();
	}
}