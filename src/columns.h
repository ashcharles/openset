#pragma once

#include "common.h"

#include <unordered_map>
#include <unordered_set>

#include "threads/locks.h"
#include "dbtypes.h"

using namespace std;

namespace openset
{
	namespace db
	{
        static const unordered_set<std::string> ColumnTypes = {
            { "int" },
            { "double" },
            { "text" },
            { "bool" }
        };

		class Columns
		{
		public:

			struct Columns_s
			{
				string name;
				int32_t idx{0};
				columnTypes_e type{ columnTypes_e::freeColumn };
				bool isSet{ false };
                bool isProp{ false };
				bool deleted{ false };
			};

			// shared lock (uses spin locks)
			CriticalSection lock;

			Columns_s columns[MAX_COLUMNS];
			unordered_map<string, Columns_s*> nameMap;
			int columnCount{ 0 };

			Columns();
			~Columns();

			// get a column record, this will always 
			// return something
			Columns_s* getColumn(int column);

			// get a column by name, this will return a nullptr
			// if none match
			Columns_s* getColumn(string name);

			void deleteColumn(Columns_s* columnInfo);

			int getColumnCount() const;

			void setColumn(
                const int index, 
                const string name, 
                const columnTypes_e type, 
                const bool isSet, 
                const bool isProp = false,
                const bool deleted = false);

            static bool validColumnName(std::string name);

		};
	};
};
