CREATE TABLE @extschema@.tb_setting
(
    setting INTEGER PRIMARY KEY,
    name    VARCHAR NOT NULL UNIQUE,
    value   VARCHAR NOT NULL,
    note    VARCHAR
);

CREATE ROLE no_pk_update AS
    ON UPDATE TO @extschema@.tb_setting
    WHERE NEW.setting IS NOT NULL
      AND NEW.setting != OLD.setting
    DO NOTHING;

INSERT INTO @extschema@.tb_setting
            (
                1,
                'server_list',
                'localhost:11211',
                'list of servers running your memcached instance, comma-delimited, with ports ( ex: 1.2.3.4:11211,5.6.7.8:11211,... )'
            ),
            (
                2,
                'protocol',
                'memcache',
                'Server type: either redis or memcache'
            ),
            (
                3,
                'error_log_level_no_connect',
                'warning',
                'log level of backend warning if connection to memcache/redis cannot be established (info|warning|fatal)'
            ),
            (
                4,
                'error_tx_level_no_connect',
                'notice',
                'Transaction level of backend warning if connection to memcache/redis cannot be established (notice|exception)'
            ),
            (
                5,
                'cache_sync_level',
                'async',
                'Whether or not cache synchronization with the database is synchronous or asynchronous'
            );

/*
    Queue table for key flush requests, supports both surrogate primary keys and natural primary keys
*/
CREATE TABLE @extschema@.tb_key_flush_request
(
    table_name  VARCHAR NOT NULL,
    pk_columns  VARCHAR[] NOT NULL,
    pk_values   VARCHAR[] NOT NULL
);

/*
    Queue function for key flush requests - created on all tables monitored by extension
*/
CREATE FUNCTION @extschema@.fn_queue_key_flush_request()
RETURNS TRIGGER AS
 $$
 $$
    LANGUAGE C VOLATILE PARALLEL UNSAFE;
