EXTENSION = gist_isax
DATA = gist_isax--1.0.sql
DOCS = README.gist_isax
MODULES = gist_isax

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)