/*
 * isax.c
 *
 *  Created on: 27Sep.,2016
 *      Author: brynj
 *
 *  Used pg_trgm module as inspiration
 */

/*
 * These functions are required by CREATE TYPE but aren't really needed, so
 * just return errors if called. You can implement them properly if you like.
 */

Datum
isax_in(PG_FUNCTION_ARGS)
{
	elog(ERROR, "not implemented");
	PG_RETURN_DATUM(0);
}

Datum
isax_out(PG_FUNCTION_ARGS)
{
	elog(ERROR, "not implemented");
	PG_RETURN_DATUM(0);
}
