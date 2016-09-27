/*
 * isax.c
 *
 *  Created on: 27Sep.,2016
 *      Author: brynj
 *
 *  Used pg_trgm module as inspiration
 */

#include "postgres.h"
#include "isax.h"
#include "utils/builtins.h"
#include <stdio.h>

/*
 * These functions allow iSAX values to be read and returned as cstrings.
 * I've chosen a format of v:c where v is the breakpoint value and c is
 * the cardinality.
 */
PG_FUNCTION_INFO_V1(isax_elem_in);
Datum
isax_elem_in(PG_FUNCTION_ARGS)
{
	/*
	 * Based upon complex_in from https://www.postgresql.org/docs/9.5/static/xtypes.html
	 */

    char       *str = PG_GETARG_CSTRING(0);
    char      x,
                y;
    ISAXELEM    *result;

    if (sscanf(str, "%hhu:%hhu", &x, &y) != 2)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for iSAX element: \"%s\"",
                        str)));

    result = (ISAXELEM *) palloc(sizeof(ISAXELEM));
    result->value = x;
    result->cardinality = y;
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(isax_elem_out);
Datum
isax_elem_out(PG_FUNCTION_ARGS)
{
	/*
	 * Based upon complex_out from https://www.postgresql.org/docs/9.5/static/xtypes.html
	 */
	ISAXELEM    *isaxelem = (ISAXELEM *) PG_GETARG_POINTER(0);
    char       *result;

    result = psprintf("%hhu:%hhu", isaxelem->value, isaxelem->cardinality);
    PG_RETURN_CSTRING(result);
}
