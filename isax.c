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
#include <stdlib.h>
#include <math.h>

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
    char		*pch;
    unsigned char      v,
				n;
    unsigned long ul, c;
    ISAXELEM    *result;

    errno = 0;
    ul = strtoul(str, &pch, 0);
    if(errno==ERANGE || ul >= MAXSAXCARDINALITY)
        ereport(ERROR,
                (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                 errmsg("iSAX element value exceeds allowed range: \"%s\"",
                        str)));
    v = (unsigned char) ul;

    if(*pch!=':')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("iSAX element is missing ':' delimiter: \"%s\"",
                        str)));

    errno = 0;
    ul = strtoul(pch+1, &pch, 0);
    if(errno==ERANGE || ul > MAXSAXCARDINALITY || ul<1)
        ereport(ERROR,
                (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                 errmsg("iSAX element cardinality exceeds allowed range: \"%s\"",
                        str)));

    c = ul;
    if (*pch!='\0')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for iSAX element: \"%s\"",
                        str)));

    /*
     * A valid cardinality must be the number of values for a given
     * number of bits, i.e., c = 2^n+1
     * 2^n = c-1
     * n = log_2(c-1)
     * Thus from http://stackoverflow.com/a/22701843/290182
     */
    n = 0;
    --c;
    while (c > 0) {
        ++n;
        c >>= 1;
    }
/*    elog(NOTICE, "Converted %s to %hhu with cardinality %lu giving %hhu bits", str, v, ul, n);*/
    result = (ISAXELEM *) palloc(sizeof(ISAXELEM));
    result->value = v;
    result->validbits = n;
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
    unsigned char      v,
				n;
    unsigned int c;
    v = isaxelem->value;
    n = isaxelem->validbits;
    c = 1 << n;
/*    elog(NOTICE, "%hhu bits gives cardinality %u", n, c);*/
    result = psprintf("%hhu:%u", v, c);
    PG_RETURN_CSTRING(result);
}
