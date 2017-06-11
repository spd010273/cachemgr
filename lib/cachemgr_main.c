/*
    Entry point for tblmgr process
*/
#include "postgres.h"

#include "miscadmin.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/lwlock.h"
#include "storage/proc.h"
#include "storage/shmem.h"


#include "access/xact.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "pgstat.h"
#include "utils/builtins.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h";

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1( worker_spi_launch );

void _PG_init( void );
void worker_spi_main( Datum ) pg_attribute_noreturn();

static volatile sig_atomic_t got_sighup = false;
static volatile sig_atomic_t got_sigterm = false;

static int worker_spi_naptime = 10;
static int worker_spi_total_workers = 2;

typedef struct worktable
{
    const char * schema;
    const char * name;
} worktable;

#define PROCESS_NAME "tblmgr_updater"

static void worker_spi_sigterm( SIGNAL_ARGS )
{
    int save_errno = errno;

    got_sigterm = true;
    SetLatch( MyLatch );

    errno = save_errno;
}


static void worker_spi_sighup( SIGNAL_ARGS )
{
    int save_errno = errno;

    got_sighup = true;
    SetLatch( MyLatch );

    errno = save_errno;
}

static void initialize_worker_spi( worktable * table )
{
    int ret;
    int ntup;
    bool isnull;
    StringInfoData buf;

    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot( GetTransactionSnapshot() );

    pgstat_report_activity( STATE_RUNNING, "Initializing Table Manager" );

    initStringInfo( &buf );
    appendStringInfo(
        &buf,
        "SELECT COUNT(*) FROM pg_namespace WHERE nspname = '%s'",
        table->schema
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

    ntup = DatumGetInt64(
        SPI_getbinval(
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

    if( ntup == 0 )
    {
    }

}
