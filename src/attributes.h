#pragma once

#include <vector>
#include "mem/bigring.h"
#include "mem/blhash.h"
#include "heapstack/heapstack.h"

#include "dbtypes.h"
#include "indexbits.h"

using namespace std;

namespace openset::db
{

    const int32_t PROP_STAMP = 0;
    const int32_t PROP_EVENT = 1;
    const int32_t PROP_UUID = 2;
    // below are fake properties used for indexing
    const int32_t PROP_SEGMENT = 5;
    const int32_t PROP_SESSION = 6;

    // user defined table properties start at this index
    const int32_t PROP_INDEX_USER_DATA = 7;

    // don't encode properties between these ranges
    const int32_t PROP_INDEX_OMIT_FIRST = PROP_UUID; // omit >=
    const int32_t PROP_INDEX_OMIT_LAST = PROP_SESSION; // omit <=

    struct BitData_s;
    class Properties;
    class Table;
    class AttributeBlob;

#pragma pack(push,1)

    struct Attr_changes_s
    {
        int32_t linId{ 0 }; // linear ID of Customer
        int32_t state{ 0 }; // 1 or 0
        Attr_changes_s* prev{ nullptr }; // tail linked.

        Attr_changes_s() = default;

        Attr_changes_s(const int32_t linId, const int32_t state, Attr_changes_s* prev) :
            linId(linId), state(state), prev(prev)
        {}
    };

    union Attr_value_u
    {
        int64_t numeric;
        char* blob; // shared location in attributes blob
    };

    class Attributes;

    /*
    attr_s defines an index item. It is cast over a variable length
    chunk of memory. "people" is an array of bytes containing
    a compressed bit index
    */
    struct Attr_s
    {
        /*
         * The Attr_s is an index structure.
         *
         * Standard layout:
         *   ints - the number of uint64_t in the bit index array decompressed
         *   comp - how much space they take compressed (in bytes)
         *
         * Sparse Layout:
         *   ints - negative number, abs value is number of int32_ts in list
         *   comp - size of array in bytes (4 x length)
         *
         * Note: some indexes are really sparce, and while they compress well
         *   if there are thousands or millions of bits, compressing just one or
         *   two bits can be a waste of processor cycles and space as the result
         *   is still larger than it need be. In situtations where the population
         *   of the index is low, we will use an array of int32_t values, where
         *   each int32_t is a linear_id (linear user id).
         *
         */
        //Attr_changes_s* changeTail{ nullptr };
        char* text{ nullptr };
        int32_t ints{ 0 }; // number of unsigned int64 integers uncompressed data uses
        int32_t ofs{ 0 };
        int32_t len{ 0 };
        int32_t comp{ 0 }; // compressed size in bytes
        int32_t linId{ -1 };
        char index[1]{ 0 }; // char* (1st byte) of packed index bits struct

        Attr_s() = default;
        IndexBits* getBits();
    };
#pragma pack(pop)

    class Attributes
    {
#pragma pack(push,1)
        struct serializedAttr_s
        {
            int32_t column;
            int64_t hashValue;
            int32_t ints; // number of int64_t's used when decompressed
            int32_t ofs;
            int32_t len;
            int32_t textSize;
            int32_t compSize;
            int32_t linId;
        };
#pragma pack(pop)

    public:

        enum class listMode_e : int32_t
        {
            EQ,
            NEQ,
            GT,
            GTE,
            LT,
            LTE,
            PRESENT
        };

        using AttrListExpanded = vector<std::pair<int64_t,Attr_s*>>; // pair, value and bits
        using AttrList = vector<Attr_s*>;

        // value and attribute info
        using ColumnIndex = bigRing<attr_key_s, Attr_s*>;
        using ChangeIndex = bigRing<attr_key_s, Attr_changes_s*>;
        using AttrPair = pair<attr_key_s, Attr_s*>;

        ColumnIndex propertyIndex{ ringHint_e::lt_5_million };
        ChangeIndex changeIndex{ ringHint_e::lt_5_million };

        Table* table;
        AttributeBlob* blob;
        Properties* properties;
        int partition;

        explicit Attributes(const int partition, Table* table, AttributeBlob* attributeBlob, Properties* properties);
        ~Attributes();

        void addChange(const int32_t propIndex, const int64_t value, const int32_t linearId, const bool state);

        Attr_s* getMake(const int32_t propIndex, const int64_t value);
        Attr_s* getMake(const int32_t propIndex, const string& value);

        Attr_s* get(const int32_t propIndex, const int64_t value) const;
        Attr_s* get(const int32_t propIndex, const string& value) const;

        void drop(const int32_t propIndex, const int64_t value);

        void setDirty(const int32_t linId, const int32_t propIndex, const int64_t value, const bool on = true);
        void clearDirty();

        // replace an indexes bits with new ones, used when generating segments
        void swap(const int32_t propIndex, const int64_t value, IndexBits* newBits) const;

        AttributeBlob* getBlob() const;

        AttrListExpanded getPropertyValues(const int32_t propIndex);
        AttrList getPropertyValues(const int32_t propIndex, const listMode_e mode, const int64_t value);

        bool operator==(const Attributes& other) const
        {
            return (partition == other.partition);
        }

        void serialize(HeapStack* mem);
        int64_t deserialize(char* mem);
    };
};

namespace std
{
    template <>
    struct hash<openset::db::Attributes>
    {
        size_t operator()(const openset::db::Attributes& x) const
        {
            return x.partition;
        }
    };
};