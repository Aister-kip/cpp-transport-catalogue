#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

namespace json_reader {

	using DataBasePtr = catalogue::TransportCatalogue*;
	using MapRenderPtr = renderer::MapRenderer*;
	using RouterPtr = transport_router::TransportRouter*;
	class JsonReader {
	public:
		explicit JsonReader(std::istream& input, DataBasePtr catalogue, MapRenderPtr map_renderer = nullptr,
			RouterPtr transport_router = nullptr);
		void PrintResult(std::ostream& output);
		void ProcessInput_();
	private:
		json::Document input_;
		DataBasePtr catalogue_ptr_;
		MapRenderPtr map_renderer_ptr_;
		RouterPtr router_ptr_;

		json::Document GetOutput();
		json::Dict GetRoutes_(Handler::RequestHandler& handler, const json::Node& request);
		json::Dict GetStops_(Handler::RequestHandler& handler, const json::Node& request);
		json::Dict GetMap_(Handler::RequestHandler& handler, const json::Node& request);
		json::Dict GetOptimalRoute_(Handler::RequestHandler& handler, const json::Node& request);
		svg::Color ProcessColor(const json::Node& color);
		void ProcessRenderSettings();
		void ProcessStops_();
		void ProcessRoutes_();
		void ProcessRouterSettings_();
		void ProcessSerialize();
		void ProcessDeserialize();

	};
}
