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
#include <string.h>
#include <math.h>

void read_isex_elem(char* str, ISAXELEM* el);
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
    ISAXELEM* result = (ISAXELEM*) palloc(sizeof(ISAXELEM));
	read_isex_elem(str, result);
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

PG_FUNCTION_INFO_V1(isax_in);
Datum
isax_in(PG_FUNCTION_ARGS)
{
    char *pch;
	char *token, *str2;
	ISAXWORD * result;
	ISAXELEM* elements;
	size_t len;
	int i;
    char       *str = PG_GETARG_CSTRING(0);
    if(*str!='{')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("iSAX word is missing '{' delimiter: \"%s\"",
                        str)));
    pch=strchr(str,'}');
    if(pch==NULL)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("iSAX word is missing '}' delimiter: \"%s\"",
                        str)));
    if(*(pch+1)!='\0')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("iSAX word has content after '}' delimiter: \"%s\"",
                        str)));

    len = pch-str-1;
    str2 = palloc(len+1);
    strncpy(str2,str+1,len);
    *(str2+len) = '\0';
	elog(NOTICE, "read isax element \"%s\" to \"%s\"", str, str2);

	result = (ISAXWORD *) palloc(sizeof(ISAXWORD));
	elements = result->elements;
	i=0;
	pch = str2;
	while ((token = strsep(&pch, ","))) {
		if(i==ISAXWORDLENGTH)
	        ereport(ERROR,
	                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
	                 errmsg("Too many elements for iSAX word: \"%s\"",
	                        str)));
		elog(NOTICE, "read isax element %s", token);
		read_isex_elem(token, elements+i++);
	}

	pfree(str2);
	pfree(str);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(isax_out);
Datum
isax_out(PG_FUNCTION_ARGS)
{
	/*
	 * Based upon complex_out from https://www.postgresql.org/docs/9.5/static/xtypes.html
	 */
	ISAXWORD    *isax = (ISAXWORD *) PG_GETARG_POINTER(0);
    char       *result;
    result = psprintf("---");
    PG_RETURN_CSTRING(result);
}

void read_isex_elem(char* str, ISAXELEM* result ) {
	char* pch;
	unsigned char v, n;
	unsigned long ul, c;
	errno = 0;
	ul = strtoul(str, &pch, 0);
	if (errno == ERANGE || ul >= MAXSAXCARDINALITY)
		ereport(ERROR,
				(errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE), errmsg(
						"iSAX element value exceeds allowed range: \"%s\"", str)));

	v = (unsigned char) ul;
	if (*pch != ':')
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), errmsg(
						"iSAX element is missing ':' delimiter: \"%s\"", str)));

	errno = 0;
	ul = strtoul(pch + 1, &pch, 0);
	if (errno == ERANGE || ul > MAXSAXCARDINALITY || ul < 1)
		ereport(ERROR,
				(errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE), errmsg(
						"iSAX element cardinality exceeds allowed range: \"%s\"",
						str)));

	c = ul;
	if (*pch != '\0')
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), errmsg(
						"invalid input syntax for iSAX element: \"%s\"", str)));

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
	result->value = v;
	result->validbits = n;
}

