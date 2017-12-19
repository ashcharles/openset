#pragma once

#include "oloop.h"
#include "person.h"

namespace openset
{
	namespace db
	{
		class Table;
		class TablePartitioned;
	};
};

namespace openset
{
	namespace async
	{

		class OpenLoopRetrigger : public OpenLoop
		{
		private:
			openset::db::Table* table;
			openset::db::Person person;
			int64_t linearId; // used as iterator 
			int64_t lowestStamp; // lowest non-expired stamp, for reschedule

		public:
			explicit OpenLoopRetrigger(openset::db::Table* table);
			~OpenLoopRetrigger() final;

			void prepare() final;
			void run() final;
			void partitionRemoved() final {};
		};
	};
};