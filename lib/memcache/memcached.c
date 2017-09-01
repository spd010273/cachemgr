#include "postgres.h"
#include "libmemcached.h"


/*
    Relies on the SPI_connect method being called to change palloc contexts
*/
#define SERVER_LIST 1

static void memcache_connect( worktable *table )
{
    StringInfoData buf;
    int ret;
    int ntup;
    bool isnull;
    char * server_list;

    /*
        Get connection parameters to memcached server(s) from the server_list entry in tb_setting
    */
    initStringInfo(&buf);

    appendStringInfo(
        &buf,
        "SELECT value FROM \"%s\".tb_setting WHERE setting = %d",
        table->schema,
        SERVER_LIST
    );

    ret = SPI_execute( buf.data, true, 0 );

    if( ret != SPI_OK_SELECT )
    {
        elog( FATAL, "SPI_execute failed: error code %d", ret );
    }

    if( SPI_processed != 1 )
    {
        elog( FATAL, "Not a singleton result" );
    }

     
    server_list = DatumGetCString(
        SPI_getvalue(
            SPI_tuptable->vals[0],
            SPI_tuptable->tupdesc,
            1,
            &isnull
        )
    );

    if( isnull )
    {
        elog( FATAL, "NULL result" );
    }

    if( strlen( server_list ) == 0 )
    {
        elog( FATAL, "No servers specified to connect to" );
    }

    
}
