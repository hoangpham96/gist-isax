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
#include "utils/array.h"

/*
 * Breakpoints for cardinality-256 iSAX elements
 * Using Anatoly Sorokin's values but may need to revise. In particular the maximum value
 * of 100.0 may not be appropriate.
 */
static float saxbp[] ={  -2.66006747, -2.41755902, -2.26622681, -2.15387469, -2.06352790, -1.98742789, -1.92135077, -1.86273187, -1.80989224, -1.76167041, -1.71722812, -1.67593972, -1.63732538, -1.60100866, -1.56668859, -1.53412054, -1.50310294, -1.47346758, -1.44507258, -1.41779714, -1.39153749, -1.36620382, -1.34171784, -1.31801090, -1.29502241, -1.27269864, -1.25099172, -1.22985876, -1.20926123, -1.18916435, -1.16953661, -1.15034938, -1.13157656, -1.11319428, -1.09518065, -1.07751557, -1.06018048, -1.04315826, -1.02643306, -1.00999017, -0.99381591, -0.97789754, -0.96222320, -0.94678176, -0.93156283, -0.91655667, -0.90175411, -0.88714656, -0.87272589, -0.85848447, -0.84441508, -0.83051088, -0.81676542, -0.80317257, -0.78972652, -0.77642176, -0.76325304, -0.75021538, -0.73730400, -0.72451438, -0.71184220, -0.69928330, -0.68683375, -0.67448975, -0.66224768, -0.65010407, -0.63805558, -0.62609901, -0.61423129, -0.60244945, -0.59075066, -0.57913216, -0.56759132, -0.55612559, -0.54473251, -0.53340971, -0.52215488, -0.51096581, -0.49984034, -0.48877641, -0.47777199, -0.46682512, -0.45593392, -0.44509652, -0.43431116, -0.42357608, -0.41288960, -0.40225007, -0.39165587, -0.38110545, -0.37059729, -0.36012989, -0.34970180, -0.33931161, -0.32895791, -0.31863936, -0.30835463, -0.29810241, -0.28788143, -0.27769044, -0.26752821, -0.25739353, -0.24728522, -0.23720211, -0.22714306, -0.21710695, -0.20709265, -0.19709908, -0.18712516, -0.17716982, -0.16723201, -0.15731068, -0.14740482, -0.13751340, -0.12763542, -0.11776987, -0.10791578, -0.09807215, -0.08823802, -0.07841241, -0.06859437, -0.05878294, -0.04897716, -0.03917609, -0.02937878, -0.01958429, -0.00979167,  0.00000000,  0.00979167,  0.01958429,  0.02937878,  0.03917609,  0.04897716,  0.05878294,  0.06859437,  0.07841241,  0.08823802,  0.09807215,  0.10791578,  0.11776987,  0.12763542,  0.13751340,  0.14740482,  0.15731068,  0.16723201,  0.17716982,  0.18712516,  0.19709908,  0.20709265,  0.21710695,  0.22714306,  0.23720211,  0.24728522,  0.25739353,  0.26752821,  0.27769044,  0.28788143,  0.29810241,  0.30835463,  0.31863936,  0.32895791,  0.33931161,  0.34970180,  0.36012989,  0.37059729,  0.38110545,  0.39165587,  0.40225007,  0.41288960,  0.42357608,  0.43431116,  0.44509652,  0.45593392,  0.46682512,  0.47777199,  0.48877641,  0.49984034,  0.51096581,  0.52215488,  0.53340971,  0.54473251,  0.55612559,  0.56759132,  0.57913216,  0.59075066,  0.60244945,  0.61423129,  0.62609901,  0.63805558,  0.65010407,  0.66224768,  0.67448975,  0.68683375,  0.69928330,  0.71184220,  0.72451438,  0.73730400,  0.75021538,  0.76325304,  0.77642176,  0.78972652,  0.80317257,  0.81676542,  0.83051088,  0.84441508,  0.85848447,  0.87272589,  0.88714656,  0.90175411,  0.91655667,  0.93156283,  0.94678176,  0.96222320,  0.97789754,  0.99381591,  1.00999017,  1.02643306,  1.04315826,  1.06018048,  1.07751557,  1.09518065,  1.11319428,  1.13157656,  1.15034938,  1.16953661,  1.18916435,  1.20926123,  1.22985876,  1.25099172,  1.27269864,  1.29502241,  1.31801090,  1.34171784,  1.36620382,  1.39153749,  1.41779714,  1.44507258,  1.47346758,  1.50310294,  1.53412054,  1.56668859,  1.60100866,  1.63732538,  1.67593972,  1.71722812,  1.76167041,  1.80989224,  1.86273187,  1.92135077,  1.98742789,  2.06352790,  2.15387469,  2.26622681,  2.41755902,  2.66006747, 100.0};

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

	result = (ISAXWORD *) palloc0(sizeof(ISAXWORD));
	elements = result->elements;
	i=0;
	pch = str2;
	while ((token = strsep(&pch, ","))) {
		if(i==ISAXWORDLENGTH)
	        ereport(ERROR,
	                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
	                 errmsg("Too many elements for iSAX word: \"%s\"",
	                        str)));
		read_isex_elem(token, elements+i++);
	}
	if(i!=ISAXWORDLENGTH)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("Too few elements for iSAX word: \"%s\"",
                        str)));
	pfree(str2);
	pfree(str);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(isax_out);
Datum
isax_out(PG_FUNCTION_ARGS)
{
	ISAXWORD    *isax = (ISAXWORD *) PG_GETARG_POINTER(0);
	ISAXELEM *isaxelem;
	size_t n = ISAXWORDLENGTH;
	size_t l;
	int i;
    char       *result,
				*pch;
    /*
     * Need to know how long the string is to allocate the right amount of memory.
     * Here I'll just guess it using an upper bound.
     * Format will be '{' value:card ',' ... value:card ',' value:card '}' '\0'
     * So for n_elems elements have 2 braces + n_elems-1 commas + n_elems iSAX value + null char
     * Max cardinality and breakpoint value is 256, each requiring 3 chars,
     * plus the ':'
     */
    l = 2 + n*8;
    result = palloc(l * sizeof(char));
    pch = result;
    *pch = '{';
    isaxelem = isax->elements;
    for(i=0; i<n; ++i) {
    	++pch;
    	pch += sprintf(pch, "%hhu:%u", isaxelem->value, 1 << isaxelem->validbits);
    	*pch = ',';
    	++isaxelem;
    }
    *pch = '}';
    *(++pch) = '\0';
    PG_RETURN_CSTRING(result);
}

/*
 * Support cast from element array to isax word
 */
PG_FUNCTION_INFO_V1(isax_elem_array_to_isax);
Datum
isax_elem_array_to_isax(PG_FUNCTION_ARGS) {
	ArrayType *a;
	int nItems;
	ISAXWORD *result;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	a = PG_GETARG_ARRAYTYPE_P(0);
	if (array_contains_nulls(a))
		ereport(ERROR,
					(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
					 errmsg("No NULLs allowed"),
					 errdetail("iSAX words cannot contain nulls.")));
	if(ARR_NDIM(a) != 1)
		ereport(ERROR,
					(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
					 errmsg("only 1D arrays allowed"),
					 errdetail("Only one-dimensional arrays can "
							   "be converted to iSAX words.")));

	nItems = ArrayGetNItems(1, ARR_DIMS(a));
	if(nItems != ISAXWORDLENGTH)
		ereport(ERROR,
					(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
					 errmsg("unsupported number of elements"),
					 errdetail("iSAX words currently only store a fixed number of elements")));

	result = (ISAXWORD *) palloc(sizeof(ISAXWORD));
	memcpy(result->elements, ARR_DATA_PTR(a), sizeof(ISAXWORD));
	PG_RETURN_POINTER(result);
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

