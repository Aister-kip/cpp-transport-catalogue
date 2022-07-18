#include "transport_router.h"

namespace transport_router {

	DistanceProcessor::DistanceProcessor(const catalogue::TransportCatalogue* catalogue, const domain::BusPtr route)
		: forward_distance_(route->stops.size())
		, reverse_distance_(route->stops.size()) 
	{
		size_t forward_accum = 0;
		size_t reverse_accum = 0;
		forward_distance_[0] = 0;
		reverse_distance_[0] = 0;
		for (size_t i = 0; i < route->stops.size() - 1; ++i) {
			forward_accum += catalogue->GetDistanceBetweenStops(route->stops[i], route->stops[i + 1]);
			forward_distance_[i + 1] = forward_accum;
			if (!route->is_circular) {
				reverse_accum += catalogue->GetDistanceBetweenStops(route->stops[i + 1], route->stops[i]);
				reverse_distance_[i + 1] = reverse_accum;
			}
		}
	}

	size_t DistanceProcessor::DistanceBetween(size_t index_from, size_t index_to) {
		if (index_from < index_to) {
			return forward_distance_[index_to] - forward_distance_[index_from];
		}
		else {
			return reverse_distance_[index_from] - reverse_distance_[index_to];
		}
	}

	TravelDuration operator+(const TravelDuration& lhs, const TravelDuration& rhs)
	{
		return { lhs.stops_number + rhs.stops_number, lhs.waiting_time + rhs.waiting_time, lhs.travel_time + rhs.travel_time };
	}

	bool operator<(const TravelDuration& lhs, const TravelDuration& rhs)
	{
		return (lhs.waiting_time + lhs.travel_time < rhs.waiting_time + rhs.travel_time);
	}

	bool operator>(const TravelDuration& lhs, const TravelDuration& rhs)
	{
		return (lhs.waiting_time + lhs.travel_time > rhs.waiting_time + rhs.travel_time);
	}

	void TransportRouter::InitRouter(RouteSettings settings, const catalogue::TransportCatalogue* catalogue) {
		settings_.bus_velocity = settings.bus_velocity / 3.6;		//Convert km/h in m/s
		settings_.bus_wait_time = settings.bus_wait_time * 60;		//Convert minutes in seconds
		catalogue_ = catalogue;
		
		auto* all_stops = catalogue_->GetStopsDeque();
		graph_ = std::make_unique<graph::DirectedWeightedGraph<TravelDuration>>(all_stops->size());
		graph::VertexId vertex_counter = 0;

		for (const auto& stop_ptr : *all_stops) {
			graph_vertexes_.insert({ &stop_ptr, vertex_counter++ });
		}

		for (const auto& route : *catalogue->GetBusesDeque()) {
			const auto& stops = route.stops;
			DistanceProcessor distance_proc(catalogue, &route);
			for (int i = 0; i < stops.size() - 1; ++i) {
				for (int j = i + 1; j < stops.size(); ++j) {
					TravelDuration travel_dur(j - i, 
						settings_.bus_wait_time, 
						distance_proc.DistanceBetween(i, j) / settings_.bus_velocity);
					TravelProps travel_unit{ stops[i], stops[j], &route, travel_dur };
					graph_->AddEdge(graph::Edge<TravelDuration>{graph_vertexes_[travel_unit.from], graph_vertexes_[travel_unit.to], travel_dur});
					graph_edges_.push_back(std::move(travel_unit));
					if (!route.is_circular) {
						TravelDuration travel_dur(j - i,
							settings_.bus_wait_time,
							distance_proc.DistanceBetween(j, i) / settings_.bus_velocity);
						TravelProps travel_unit{ stops[j], stops[i], &route, travel_dur };
						graph_->AddEdge(graph::Edge<TravelDuration>{graph_vertexes_[travel_unit.from], graph_vertexes_[travel_unit.to], travel_dur});
						graph_edges_.push_back(std::move(travel_unit));
					}
				}
			}
		}
		router_ = std::make_unique<graph::Router<TravelDuration>>(*graph_);
	}

	void TransportRouter::InitRouter(RouteSettings settings, const catalogue::TransportCatalogue* catalogue,
		std::unique_ptr<graph::DirectedWeightedGraph<TravelDuration>> graph,
		std::unordered_map<domain::StopPtr, graph::VertexId> graph_vertexes,
		std::vector<TravelProps> graph_edges,
		std::unique_ptr<graph::Router<TravelDuration>> router) {
		settings_ = settings;
		catalogue_ = catalogue;
		graph_.swap(graph);
		graph_vertexes_ = graph_vertexes;
		graph_edges_ = graph_edges;
		router_.swap(router);
	}

	std::optional<std::vector<const TravelProps*>> TransportRouter::FindRoute(std::string_view from, std::string_view to) const {
		catalogue::StopPtr stop_from = catalogue_->FindStop(from);
		catalogue::StopPtr stop_to = catalogue_->FindStop(to);
		if (stop_from == nullptr || stop_to == nullptr) {
			return std::nullopt;
		}
		std::vector<const TravelProps*> result;
		if (stop_from == stop_to) {
			return result;
		}
		graph::VertexId vertex_from = graph_vertexes_.at(stop_from);
		graph::VertexId vertex_to = graph_vertexes_.at(stop_to);
		auto route = router_->BuildRoute(vertex_from, vertex_to);
		if (!route.has_value()) {
			return std::nullopt;
		}
		for (const auto& edge : route.value().edges) {
			result.push_back(&graph_edges_.at(edge));
		}
		return result;
	}

	const RouteSettings& TransportRouter::GetSettings() const {
		return settings_;
	}

	const graph::DirectedWeightedGraph<TravelDuration>& TransportRouter::GetGraph() const {
		return *graph_;
	}

	const std::unordered_map<domain::StopPtr, graph::VertexId>& TransportRouter::GetVertexes() const {
		return graph_vertexes_;
	}

	const std::vector<TravelProps>& TransportRouter::GetEdges() const {
		return graph_edges_;
	}
	const graph::Router<TravelDuration>& TransportRouter::GetRouter() const {
		return *router_;
	}
}
