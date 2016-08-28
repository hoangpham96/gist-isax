-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION gist_isax" to load this file. \quit

CREATE FUNCTION array_dist(anyarray, anyarray)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

-- CREATE FUNCTION arrays_similar(anyarray, anyarray)
-- RETURNS bool
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OPERATOR % (LEFTARG = anyarray, RIGHTARG = anyarray, PROCEDURE = arrays_similar);

-- CREATE OR REPLACE FUNCTION my_consistent(internal, data_type, smallint, oid, internal)
-- RETURNS bool
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_union(internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_compress(internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_decompress(internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_penalty(internal, internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_picksplit(internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_same(internal, internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION my_distance(internal, data_type, smallint, oid)
-- RETURNS float8
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;
