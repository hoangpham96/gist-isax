#include "postgres.h"
/* Array handling functions */
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "fmgr.h"
#include "timeseries.h"
#include <float.h>
#include <math.h>
/*
 * The functions here are written to conform with the Postgres function manager and
 * function-call interface. See http://doxygen.postgresql.org/fmgr_8h_source.html
 */

PG_MODULE_MAGIC;

#define ARRPTR(x)  ( (float4 *) ARR_DATA_PTR(x) )

static inline bool
ATTR_IS_FLOAT4(Oid typid) {
 return typid == FLOAT4OID;
}

/*
 * GUC variables
 */
double distance_threshold = 0.5f;

void _PG_init(void);

/*
 * Module load callback - called immediately after loading file
 */
void _PG_init(void)
{
	/* Define custom GUC variables
	 * see http://doxygen.postgresql.org/guc_8h_source.html
	 */
	DefineCustomRealVariable("gist_isax.distance_threshold",
	  "Sets the threshold used by the %% operator.",
		"Valid range is any positive real number.",
		&distance_threshold,
		0.5f,
		0.0,
		DBL_MAX,
		PGC_USERSET,
		0,
		NULL,
		NULL,
		NULL);
}


static void
check_elem_types(Oid element_type1, Oid element_type2){
		/* Expect arrays of same length and type */
		if (element_type1 != element_type2)
			ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("cannot compare incompatible arrays"),
				 errdetail("Arrays with element types %s and %s are not "
						  "compatible for distance measurement.",
						  format_type_be(element_type1),
							format_type_be(element_type2))));

		/* Also make the simplification of only handling floats */
		if(!ATTR_IS_FLOAT4(element_type1) || !ATTR_IS_FLOAT4(element_type2))
		{
			ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("must have real type arrays"),
				 errdetail("Arrays with element types %s and %s are not "
						  "compatible current implementation of distance measurement.",
						  format_type_be(element_type1),
							format_type_be(element_type2))));
		}
}

static void
check_ndims(int ndims1, int ndims2)
{
		if(ndims1 != 1 || ndims2 != 1)
		{
			ereport(ERROR,
						(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
						 errmsg("only 1D arrays allowed"),
						 errdetail("Only one-dimensional arrays can "
								   "be compared for similarity.")));
		}
}

static void
check_nitems(int nitems1, int nitems2)
{
		if(nitems1 != nitems2)
		{
			ereport(ERROR,
						(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
						 errmsg("arrays must be same length"),
						 errdetail("Arrays with lengths %d and %d are not "
						  "compatible for distance measurement.",
							nitems1, nitems2)));
		}
}

PG_FUNCTION_INFO_V1(arrays_similar);
Datum arrays_similar(PG_FUNCTION_ARGS);

/*
 * Determine whether two arrays have a Euclidean Distance less than the distance_threshold
 * This function can provide early termination.
 *
 * For other examples of handling arrays check src/backend/utils/adt/array_userfuncs.c
 *
 * The returned result should be true if the distance is lower than distance_threshold, else false
 */
Datum
arrays_similar(PG_FUNCTION_ARGS)
{
		ArrayType		*v1,
								*v2;
		int					*dims1,
								ndims1,
								nitems1;
		int					*dims2,
								ndims2,
								nitems2;
		Oid					element_type1;
		Oid					element_type2;
		bool 				is_similar = true;
    float4			distance = 0;
    int         i;
    float4      *val1, *val2;
    float4      delta;

		/* Can't do anything with null arrays */
		if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
		{
			PG_RETURN_NULL();
		}

		v1 = PG_GETARG_ARRAYTYPE_P(0);/* NOTE: Use PG_GETARG_ARRAYTYPE_P_COPY if need to alter the values */
		v2 = PG_GETARG_ARRAYTYPE_P(1);

		/* Also don't want nulls within the array */
		if (array_contains_nulls(v1) || array_contains_nulls(v2))
		{
			PG_RETURN_NULL();
		}

		element_type1 = ARR_ELEMTYPE(v1);
		element_type2 = ARR_ELEMTYPE(v2);

		check_elem_types(element_type1, element_type2);

		/* Only deal with 1D arrays */
		ndims1 = ARR_NDIM(v1);
		ndims2 = ARR_NDIM(v2);
		check_ndims(ndims1, ndims2);

		/* Only deal with arrays of the same length */
		dims1 = ARR_DIMS(v1);
		dims2 = ARR_DIMS(v2);
		nitems1 = ArrayGetNItems(ndims1, dims1);
		nitems2 = ArrayGetNItems(ndims2, dims2);
		check_nitems(nitems1, nitems2);

		/* c.f. _int_same(PG_FUNCTION_ARGS) in contrib/intarray/_int_op.c */
		/*
		 * TODO: Implement checks
		 */
    val1 = ARRPTR(v1);
    val2 = ARRPTR(v2);
    for(i=0; i<nitems1; i++)
    {
    	delta = *val2 - *val1;
    	distance += delta * delta;
      if(distance>distance_threshold){
        is_similar = false;
        break;
      }
    	val1++;
    	val2++;
    }

	PG_RETURN_BOOL(is_similar);
}

PG_FUNCTION_INFO_V1(array_dist);
Datum array_dist(PG_FUNCTION_ARGS);

Datum
array_dist(PG_FUNCTION_ARGS)
{
		ArrayType		*v1,
								*v2;
		int					*dims1,
								ndims1,
								nitems1;
		int					*dims2,
								ndims2,
								nitems2;
		Oid					element_type1;
		Oid					element_type2;
		float4			distance = 0;
		int i;
		float4 *val1, *val2;
		float4 delta;

		/* Can't do anything with null arrays */
		if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
		{
			PG_RETURN_NULL();
		}

		v1 = PG_GETARG_ARRAYTYPE_P(0);
		v2 = PG_GETARG_ARRAYTYPE_P(1);

		/* Also don't want nulls within the array */
		if (array_contains_nulls(v1) || array_contains_nulls(v2))
		{
			PG_RETURN_NULL();
		}

		element_type1 = ARR_ELEMTYPE(v1);
		element_type2 = ARR_ELEMTYPE(v2);
		check_elem_types(element_type1, element_type2);

		/* Only deal with 1D arrays */
		ndims1 = ARR_NDIM(v1);
		ndims2 = ARR_NDIM(v2);
		check_ndims(ndims1, ndims2);

		/* Only deal with arrays of the same length */
		dims1 = ARR_DIMS(v1);
		dims2 = ARR_DIMS(v2);
		nitems1 = ArrayGetNItems(ndims1, dims1);
		nitems2 = ArrayGetNItems(ndims2, dims2);
		check_nitems(nitems1, nitems2);

		/* c.f. _int_same(PG_FUNCTION_ARGS) in contrib/intarray/_int_op.c */
		/*
		 * TODO: Implement checks
		 */
		val1 = ARRPTR(v1);
		val2 = ARRPTR(v2);
		for(i=0; i<nitems1; i++)
		{
			delta = *val2 - *val1;
			distance += delta * delta;
			val1++;
			val2++;
		}

	PG_RETURN_FLOAT4(sqrt(distance));
}


PG_FUNCTION_INFO_V1(ts_to_paa);
Datum ts_to_paa(PG_FUNCTION_ARGS);

Datum
ts_to_paa(PG_FUNCTION_ARGS)
{
  char* ts = PG_GETARG_CSTRING(0);
  char* result;

  char test[] = ts;
  result = ts;

	PG_RETURN_CSTRING(result);
}
