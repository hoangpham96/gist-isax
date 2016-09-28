EXTENSION = gist_isax
OBJS = timeseries.o isax.o # isax_gist.o
DATA = gist_isax--1.0.sql
DOCS = README.gist_isax
MODULE_big = gist_isax
REGRESS = timeseries_ops_test isax_ops_test

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
