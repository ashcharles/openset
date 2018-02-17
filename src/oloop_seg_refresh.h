#pragma once
#include "common.h"
#include "database.h"
#include "oloop.h"
#include "shuttle.h"
#include "querycommon.h"
#include "queryindexing.h"
#include "queryinterpreter.h"
#include "result.h"

namespace openset
{
	namespace db
	{
		class Table;
		class TablePartitioned;
	};

	namespace async
	{
		class OpenLoopSegmentRefresh : public OpenLoop
		{
		public:
			openset::db::TablePartitioned* parts;
			openset::db::Database::TablePtr table;

			int64_t maxLinearId;
			int64_t currentLinId;
			Person person;
			openset::query::Interpreter* interpreter;
			int instance;
			int runCount;

			openset::query::Indexing indexing;
			openset::db::IndexBits* index;

			std::string segmentName;
			query::Macro_s macros;
			std::string resultName;

			explicit OpenLoopSegmentRefresh(openset::db::Database::TablePtr table);

			~OpenLoopSegmentRefresh() final;

			// store segments that have a TTL
			void storeSegment(IndexBits* bits) const;

			bool nextExpired();

			void prepare() final;
		    void respawn();
		    void run() final;
			void partitionRemoved() final {};
		};
	}
}
