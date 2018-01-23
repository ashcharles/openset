# Cluster

## PUT /v1/cluster/init?partitions={#}

Initializes a cluster (a cluster with just **one** node will still need initializing). 

This turns an unassigned/waiting node into a cluster sentinel. A node that is part of a cluster can invite other nodes to join that cluster.

`partitions` is specifies the number of shards the data will be divided into. Pick a number that is roughly 10-20% higher than the totals number of cores your envision being part of your final cluster. 

Returns a 200 or 400 status code.

> :pushpin:the ideal partition size is the lowest possible number that will fit the size of your cluster in the long run. There is overhead incurred with each partition, but you also want to pick a number that will allow you to grow. Picking a number less than the number of processor cores in your cluster will __not__ allow you to reach peak performance.

## PUT /v1/cluster/join?host={host|ip}&port={port}

__query_params:__
- host={name | ip}
- port={port} (optional, default is 8080)

This will issue a `/v1/internode/is_cluster_member` request, and if that returns `false` a `/v1/internode/join_to_cluster` request.

Returns a 200 or 400 status code.

## Table

## POST /v1/table/{table} (create table)

```json
{
    "columns": [
        {
            "name": "{column_name}",
            "type": "{text|int|double|bool}",           
            "is_set": {optional: true|false}
        },
        {
            "name": "{column_name}",
            "type": "{text|int|double|bool}",
            "is_set": {optional: true|false}
        },
        //etc        
    ],
    "z-order": [
        "{event_name}",
        "{event_name}",
        //etc
    ]
}
```

Returns a 200 or 400 status code.

## GET /v1/table/{table} (describe table)

Returns JSON describing the table.

```json
{
    "table": "highstreet",
    "columns": [
        {
            "name": "product_name",
            "type": "text"
        },
        {
            "name": "product_price",
            "type": "double"
        },
        {
            "name": "product_shipping",
            "type": "double"
        },
        {
            "name": "shipper",
            "type": "text"
        },
        {
            "name": "total",
            "type": "double"
        },
        {
            "name": "shipping",
            "type": "double"
        },
        {
            "name": "product_tags",
            "type": "text",
            "is_set": true
        },
        {
            "name": "product_group",
            "type": "text"
        },
        {
            "name": "cart_size",
            "type": "int"
        }
    ]
}
```

- `type` can be `text|int|double|bool`. 
- `name` can be any string consisting of lowercase letters `a-z`, numbers `0-9`, or the `_`. Column cannot start with number.

Returns a 200 or 400 status code.

## PUT /v1/table/{table}/column/{column_name}?type={type}&is_set={is_set}

Adds a column to an existing table. 

- `type` can be `text|int|double|bool`. 
- `is_set` (optional) can be `true|false` and indicates that the column can contain multiple values per row.
- `column_name` can be any string consisting of lowercase letters `a-z`, numbers `0-9`, or the `_`. Column cannot start with number.

Returns a 200 or 400 status code.

## DELETE /v1/table/{table}/column/{column_name} 

Removes a column from the table. 

- `column_name` can be any string consisting of lowercase letters `a-z`, numbers `0-9`, or the `_`. Column cannot start with number.

Returns a 200 or 400 status code.

## POST /v1/revent/{table}/trigger/{trigger_name}

Create a re-eventing trigger.

## GET /v1/revent/{table}/trigger/{trigger_name}

Describe a re-eventing trigger

## GET /v1/revent/{table}/trigger/{trigger_name}/sub/{sub_name}

Subscribe to a trigger (map a web-hook)

# DELETE /v1/revent/{table}/trigger/{trigger_name}

Describe a re-eventing trigger

# Query

## POST /v1/query/{table}/event

This will perform an event scanning query by executing the provided `PyQL` script in the POST body as `text/plain`. The result will be in JSON and contain results or any errors produced by the query.

**query parameters:**

| param | values            | note |
| ---- | ----------------- | ---- |
|`debug=`| `true/false`      | will return the assembly for the query rather than the results|
| `segments=`| `segment,segment` | comma separted segment list. Segment must be created with a `counts` query. The segment `*` represents all people. |
|`sort=`| `column_name`     | sort by column name.|
|`order=`| `asc/desc`        | default is descending order.|
|`trim=`| `# limit`         | clip long branches at a certain count. Root nodes will still include totals for the entire branch. |
|`str_{var_name}` | `text`            | replace `{{var_name}}` string in script (will be automatically quoted)|
|`int_{var_name}` | `integer`         | replace `{{var_name}}` numeric value in script|
|`dbl_{var_name}` | `double`          | replace `{{var_name}}` numeric value in script|
|`bool_{var_name}` | `true/false`      | replace `{{var_name}}` boolean value in script|

**result**

200 or 400 status with JSON data or error.

## POST /v1/query/{table}/segment

This will perform an index counting query by executing the provided `PyQL` script in the POST body as `text/plain`. The result will be in JSON and contain results or any errors produced by the query.

Unlike the `/events` query, segments created in by the `/counts` query are named and cached and can be used in subsequent `/counts` queries as well as to filter `/events` queries.

A single counts query can contain multiple sections to create multiple segments in one step.

**query parameters:**

| param | values | note |
| ---- | ---- | ---- |
|`debug=` | `true/false` |  will return the assembly for the query rather than the results|

**post body:**

The post body can include multiple sections. We've borrowed the python `@` decorator to define sections. The example below is using the sample `high_street` sample data to create two segments named `products_home` and `products_outdoor`. 

The `params` on the `@segment` definition tell OpenSet to not-recalculate the segment if it's within the TTL, and that it's ok to use a cached version. It also tells OpenSet to refresh this segment about every 300 seconds.

> :pushpin: These segments can be fully derived by counting indexes and are very fast. Adding sequence or time constraints will result in the segment generator executing the script against the event set (OpenSet will automatically determine if this is required). Event based segments will be slower if they cover a large cross-section of people in the database. Once segments are created they can be used by name in the `segments=` parameter of other queries.

```
@segment products_home ttl=300s use_cached refresh=300s:
    # match one of these
    match where product_group in ['basement', 'garage', 'kitchen', 'bedroom', 'bathroom']:
        tally

@segment products_outdoor ttl=300s use_cached refresh=300s:
    # match one of these
    match where product_group in ['outdoor', 'angling']:
        tally
```

**result:**

200 or 400 status with JSON data or error.

## GET /v1/query/{table}/column/{column_name}

The column query allows you to query all the values within a named column in a table as well as perform searches and numeric grouping.

**query parameters:**

| param | values              | note |
| ---- | --------------------| ---- |
| `segments=` | `segment,segment`   | comma separted segment list. Segment must be created with a `counts` query. The segment `*` represents all people. |
|`order=` | `asc/desc`          |  default is descending order. |
|`trim=` | `# limit`           | clip long branches at a certain count. Root nodes will still include totals for the entire branch. |
|`gt=` | `#`                 | return values greater than `#` |
|`gte=` | `#`                 | return values greater than or equal to `#` |
|`lt=` | `#`                 | return values less than `#` |
|`lte=` | `#`                 | return values less than or equal to `#` |
|`eq=` | `#`                 | return value equal to `#` |
|`between=` `and=` | `#` and `#`         | return value greater than or equal to `#` and less than the second number provided by `and=`|
|`rx=` | `regular expression`| return values matching an`ECMAScript` style regex expression. Check this great [cheet sheet](http://cpprocks.com/regex-cheatsheet/). |
|`sub=` | `text`              | return value containing the search string in `sub` |
|`bucket=` | `#`                 |  cluster values by `#`,  all user counts will remain distinct for each group. Useful for creating histograms or condensing results (i.e. values to nearest dollar) |

**result**

200 or 400 status with JSON data or error.

## GET /v1/query/{table}/person

Returns the event sequence for an individual. 

> :pushpin: If events contain complex data (i.e. sub values), OpenSet will re-condense the data by folding up data permeations generated on insert. The folded row may be grouped differently than the one provided to `/insert` but will be logically identical. 

**query parameters:**

| param | values  | note |
| --- | -------- | ------------------- |
| `sid=` | `string` | If you are using textual IDs use the `sid=` parameter |
|`id=` | `number` | If you are using numeric IDs use the `id=` parameter |

**result**

200 or 400 status with JSON data or error.

## POST /v1/query/{table}/histogram/{name}

This will generate a histogram using`PyQL` script in the POST body as `text/plain`. The result will be in JSON and contain results or any errors produced by the query.

If `bucket=` is provided in the query, then missing values will be zero-filled. The default is to fill between the min and max value in the set. `min=` can be used to override minimum fill. `max=` can be used to override maximum fill. If `max=` is provided, all values over the max will be totaled into the value of max.

**URL**
* `table`  is the name of the table to query
* `name` is the name of the dataset in the resulting JSON.

**query parameters:**

| param | values            | note |
| ---- | ----------------- | ---- |
|`debug=`| `true/false`      | will return the assembly for the query rather than the results|
| `segments=`| `segment,segment` | comma separted segment list. Segment must be created with a `counts` query. The segment `*` represents all people. |
|`order=`| `asc/desc`        | default is descending order.|
|`trim=`| `# limit`         | clip long branches at a certain count. Root nodes will still include totals for the entire branch. |
|`str_{var_name}` | `text`            | replace `{{var_name}}` string in script (will be automatically quoted)|
|`int_{var_name}` | `integer`         | replace `{{var_name}}` numeric value in script|
|`dbl_{var_name}` | `double`          | replace `{{var_name}}` numeric value in script|
|`bool_{var_name}` | `true/false`      | replace `{{var_name}}` boolean value in script|
|`bucket=` | `#`                 |  cluster values by `#`, all user counts
|`min=` | `#`                 |  set histogram fill to `min=#`
|`max=` | `#`                 |  clip histogram fill at `max=#`. The value in max will contain the sum of all nodes `>=` to the `max=` value.

**result**

200 or 400 status with JSON data or error.

## POST /v1/query/{table}/batch (experimental)

Run multiple segment, column and histogram queries at once, generate a single result. Including `foreach` on histograms.

Example post data using `highstreet` sample data:
```python
@segment products_home ttl=300s use_cached refresh=300s:
    # match one of these
    match where product_group in ['basement', 'garage', 'kitchen', 'bedroom', 'bathroom']:
        tally

@segment products_outdoor ttl=300s use_cached refresh=300s:
    # match one of these
    match where product_group in ['outdoor', 'angling']:
        tally

@use products_home,products_outdoor

@column product_name

@column product_group

@column product_price bucket=50

@histogram customer_value bucket=50

    return SUM total

@histogram days_since

    return int(to_days(now - last_event) / 7)

@histogram total_by_shipper foreach=shipper bucket=100 min=0 max=1000

    return SUM total where shipper=each_value # inline aggregation
```

# Internode (internode node chatter)

Don't call these from client code. 
The `/v1/internode` REST interface is used internally to maintain a proper functioning cluster. 

## GET /v1/cluster/is_member

This will return a JSON object informing if the node is already part of a cluster

```
{
    "part_of_cluster": {true|false}
}
```

## POST /v1/internode/join_to_cluster

Joins an empty node to the cluster. This originates with the `/v1/cluster/join` endpoint. `/v1/cluster/join` will issue a `/v1/interndoe/is_cluster_member` and verify the certificate before this endpoint (`/v1/internode/join_to_cluster`) is called.

This endpoint transfers information about tables, triggers, and partition mapping.

## POST /v1/internode/add_node

Dispatched to all nodes by ` /v1/cluster/join` to inform all nodes in the cluster that a new node has been joined to the cluster. Nodes receiving `add_node` will adjust their node mapping. 

At this point the node will be empty. The `sentinel` for the elected node will start balancing to this node shortly after this dispatch.

## POST /v1/internode/map_change

Dispatched by `sentinel` when node mapping and membership have changed. This is the basic mechanism that keeps cluster topology in sync.

## PUT /v1/internode/transfer?partition={partition_id}&node={dest_node_name}

This initiates a partition transfer. The node containing the partition to transfer is contacted directly. It is provided the `partition_id` to transfer and the `dest_node_name` to send it to. 

This will result in potentially several transfers, one for each table using `POST /v1/internode/transfer`. The recipient receives `partition_id` and `table_name` for each block.

After a successful transfer the `sentinel` will send a `POST /v1/internode/map_change` request to tell the cluster that the partition is available. 

## POST /v1/internode/transfer?partition={partition_id}&table={table_name}

Transfers packed `binary` data for partition. Partition is `partition_id` is passed in URL as an integer.

# Other

## GET /ping

If the node is runing, this will respond with 200 OK and JSON:
```json
{
  "pong": true
}
```



