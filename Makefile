MODULES = cachemgr
EXTENSION = cachemgr
DATA = sql/cachemgr--0.1.sql
PGFILEDESC = "cachemgr - DB to Caching integration"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif
