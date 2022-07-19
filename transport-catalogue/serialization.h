#pragma once
#include <transport_catalogue.pb.h>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <fstream>

namespace proto_serialize {
	using DataBasePtr = catalogue::TransportCatalogue*;
	using MapRenderPtr = renderer::MapRenderer*;
	using RouterPtr = transport_router::TransportRouter*;
	class Serialize {
	public:
		explicit Serialize(DataBasePtr tc_ptr, MapRenderPtr mp_ptr = nullptr, RouterPtr tr_ptr = nullptr)
			: tc_ptr_(tc_ptr)
			, mp_ptr_(mp_ptr)
			, tr_ptr_(tr_ptr) {
		};

		void CatalogueSerialize();
		void RendererSerialize();
		void RouterSerialize();
		void SaveToFile(std::string& filename);

	private:
		DataBasePtr tc_ptr_;
		MapRenderPtr mp_ptr_;
		RouterPtr tr_ptr_;
		tc_serialize::TransportCatalogue proto_catalogue_;

		void ProcessColor(tc_serialize::Color& proto_color, const svg::Color& color);
		tc_serialize::Graph ProcessGraph();
		tc_serialize::Router ProcessRouter();
	};

	class Deserialize {
	public:
		Deserialize() = delete;
		explicit Deserialize(DataBasePtr tc_ptr, MapRenderPtr mp_ptr = nullptr, RouterPtr tr_ptr = nullptr)
			: tc_ptr_(tc_ptr)
			, mp_ptr_(mp_ptr) 
			, tr_ptr_(tr_ptr) {
		};

		void CatalogueDeserialize();
		void RendererDeserialize();
		void RouterDeserialize();
		void ReadFromFile(std::string& filename);
	private:
		DataBasePtr tc_ptr_;
		MapRenderPtr mp_ptr_;
		RouterPtr tr_ptr_;
		tc_serialize::TransportCatalogue proto_catalogue_;

		svg::Color ProcessColor(const tc_serialize::Color& proto_color) const;
		std::unique_ptr<graph::DirectedWeightedGraph<transport_router::TravelDuration>> ProcessGraph();
		std::unique_ptr<graph::Router<transport_router::TravelDuration>> ProcessRouter(graph::DirectedWeightedGraph<transport_router::TravelDuration>& graph);
	};
}