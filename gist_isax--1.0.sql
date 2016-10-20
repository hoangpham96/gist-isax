-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION gist_isax" to load this file. \quit

CREATE FUNCTION array_dist(anyarray, anyarray)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION arrays_similar(anyarray, anyarray)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION ts_to_paa(anyarray)
RETURNS anyarray
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION paa_to_isax(anyarray)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION calc_lower_bp(int, int)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION calc_upper_bp(int, int)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

-- DROP EXTENSION gist_isax CASCADE;
-- CREATE EXTENSION gist_isax;

CREATE OPERATOR % (LEFTARG = anyarray, RIGHTARG = anyarray, PROCEDURE = arrays_similar);

--
-- iSAX type (for GiST keys)
--

CREATE FUNCTION isax_elem_in(cstring)
RETURNS isaxelem
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION isax_elem_out(isaxelem)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE isaxelem (
        INTERNALLENGTH = 2,
        INPUT = isax_elem_in,
        OUTPUT = isax_elem_out
);

CREATE FUNCTION isax_in(cstring)
RETURNS isax
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION isax_out(isax)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE isax (
        INTERNALLENGTH = 28,
        INPUT = isax_in,
        OUTPUT = isax_out,
        ELEMENT = isaxelem
);

CREATE FUNCTION isax(isaxelem[])
RETURNS isax
AS 'MODULE_PATHNAME', 'isax_elem_array_to_isax'
LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (isaxelem[] AS isax) WITH FUNCTION isax(isaxelem[]);

--
-- GiST functions
--

-- CREATE OR REPLACE FUNCTION gist_isax_consistent(internal, data_type, smallint, oid, internal)
-- RETURNS bool
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_union(internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_compress(internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_decompress(internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_penalty(internal, internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_picksplit(internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_same(internal, internal, internal)
-- RETURNS internal
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

-- CREATE OR REPLACE FUNCTION gist_isax_distance(internal, data_type, smallint, oid)
-- RETURNS float8
-- AS 'MODULE_PATHNAME'
-- LANGUAGE C STRICT;

--CREATE OPERATOR CLASS gist_isax_ops
--FOR TYPE anyarray USING gist
--AS
--        OPERATOR        1       % (anyarray, anyarray),
--        FUNCTION        1       gist_isax_consistent (internal, text, smallint, oid, internal),
--        FUNCTION        2       gist_isax_union (internal, internal),
--        FUNCTION        3       gist_isax_compress (internal),
--        FUNCTION        4       gist_isax_decompress (internal),
--        FUNCTION        5       gist_isax_penalty (internal, internal, internal),
--        FUNCTION        6       gist_isax_picksplit (internal, internal),
--        FUNCTION        7       gist_isax_same (isax, isax, internal),
--STORAGE isax_;
