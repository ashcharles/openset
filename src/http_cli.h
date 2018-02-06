#pragma once

#include "common.h"
#include "http_serve.h"
#include "client_http.hpp"
#include "utility.hpp"
#include "sba/sba.h"
#include "cjson/cjson.h"
#include "threads/locks.h"
#include "threads/spinlock.h"

namespace openset
{
	namespace http = SimpleWeb;
}

namespace openset::web
{
	using RestCbJson = std::function<void(const http::StatusCode, const bool, const cjson)>;
	using RestCbBin = std::function<void(const http::StatusCode, const bool, char*, const size_t)>;
	using HttpClient = SimpleWeb::Client<http::HTTP>;
	using QueryParams = http::CaseInsensitiveMultimap;

	class Rest
	{
		CriticalSection cs;
		HttpClient client;
        string host;
        int64_t routeId;

		std::string makeParams(const QueryParams params) const;

	public:

		Rest(const int64_t routeId, const std::string server) :
			client(server),
            host(server),
            routeId(routeId)
		{
			//cout << "****" << server << endl;
			//client.io_service = openset::globals::global_io_service;
		};

		~Rest()
		{
			//cout << "destroyed" << endl;
		}

        int64_t getRouteId() const
		{
		    return routeId;
		}

		void request(const std::string& method, const std::string& path, const QueryParams& params, const char* payload,
		             const size_t length, RestCbJson cb);;

		void request(const std::string& method, const std::string& path, const QueryParams& params, const char* payload,
		             const size_t length, RestCbBin cb);;

	};

	using RestPtr = shared_ptr<Rest>;
};	