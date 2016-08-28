EXTENSION = gist_isax
OBJS = gist_isax.o
DATA = gist_isax--1.0.sql
DOCS = README.gist_isax
MODULES = gist_isax
REGRESS = gist_isax_test

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
