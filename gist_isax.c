#include "postgres.h"
/* Array handling functions */
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "fmgr.h"
#include "gist_isax.h"
#include <float.h>

/*
 * The functions here are written to conform with the Postgres function manager and 
 * function-call interface. See http://doxygen.postgresql.org/fmgr_8h_source.html
 */
 
PG_MODULE_MAGIC;

static inline bool
ATTR_IS_FLOAT4(Oid typid) {
 return typid == FLOAT4OID;
} 

/*
 * GUC variables
 */
double distance_threshold = 0.5f;

void _PG_INIT(void);

/*
 * Module load callback - called immediately after loading file
 */
void _PG_INIT(void)
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
							nitems1, nitems1)));
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
		 

	PG_RETURN_FLOAT4(distance);
}

/**
 * GiST functions to be implemented
 */
 
// PG_FUNCTION_INFO_V1(my_consistent);

// Datum
// my_consistent(PG_FUNCTION_ARGS)
// {
    // GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
    // data_type  *query = PG_GETARG_DATA_TYPE_P(1);
    // StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
    // /* Oid subtype = PG_GETARG_OID(3); */
    // bool       *recheck = (bool *) PG_GETARG_POINTER(4);
    // data_type  *key = DatumGetDataType(entry->key);
    // bool        retval;

    // /*
     // * determine return value as a function of strategy, key and query.
     // *
     // * Use GIST_LEAF(entry) to know where you're called in the index tree,
     // * which comes handy when supporting the = operator for example (you could
     // * check for non empty union() in non-leaf nodes and equality in leaf
     // * nodes).
     // */

    // *recheck = true;        /* or false if check is exact */

    // PG_RETURN_BOOL(retval);
// }

// PG_FUNCTION_INFO_V1(my_union);

// Datum
// my_union(PG_FUNCTION_ARGS)
// {
    // GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
    // GISTENTRY  *ent = entryvec->vector;
    // data_type  *out,
               // *tmp,
               // *old;
    // int         numranges,
                // i = 0;

    // numranges = entryvec->n;
    // tmp = DatumGetDataType(ent[0].key);
    // out = tmp;

    // if (numranges == 1)
    // {
        // out = data_type_deep_copy(tmp);

        // PG_RETURN_DATA_TYPE_P(out);
    // }

    // for (i = 1; i < numranges; i++)
    // {
        // old = out;
        // tmp = DatumGetDataType(ent[i].key);
        // out = my_union_implementation(out, tmp);
    // }

    // PG_RETURN_DATA_TYPE_P(out);
// }

// PG_FUNCTION_INFO_V1(my_compress);

// Datum
// my_compress(PG_FUNCTION_ARGS)
// {
    // GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
    // GISTENTRY  *retval;

    // if (entry->leafkey)
    // {
        // /* replace entry->key with a compressed version */
        // compressed_data_type *compressed_data = palloc(sizeof(compressed_data_type));

        // /* fill *compressed_data from entry->key ... */

        // retval = palloc(sizeof(GISTENTRY));
        // gistentryinit(*retval, PointerGetDatum(compressed_data),
                      // entry->rel, entry->page, entry->offset, FALSE);
    // }
    // else
    // {
        // /* typically we needn't do anything with non-leaf entries */
        // retval = entry;
    // }

    // PG_RETURN_POINTER(retval);
// }

// PG_FUNCTION_INFO_V1(my_decompress);

// Datum
// my_decompress(PG_FUNCTION_ARGS)
// {
    // PG_RETURN_POINTER(PG_GETARG_POINTER(0));
// }

// PG_FUNCTION_INFO_V1(my_penalty);

// Datum
// my_penalty(PG_FUNCTION_ARGS)
// {
    // GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
    // GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
    // float      *penalty = (float *) PG_GETARG_POINTER(2);
    // data_type  *orig = DatumGetDataType(origentry->key);
    // data_type  *new = DatumGetDataType(newentry->key);

    // *penalty = my_penalty_implementation(orig, new);
    // PG_RETURN_POINTER(penalty);
// }

// PG_FUNCTION_INFO_V1(my_picksplit);

// Datum
// my_picksplit(PG_FUNCTION_ARGS)
// {
    // GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
    // OffsetNumber maxoff = entryvec->n - 1;
    // GISTENTRY  *ent = entryvec->vector;
    // GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
    // int         i,
                // nbytes;
    // OffsetNumber *left,
               // *right;
    // data_type  *tmp_union;
    // data_type  *unionL;
    // data_type  *unionR;
    // GISTENTRY **raw_entryvec;

    // maxoff = entryvec->n - 1;
    // nbytes = (maxoff + 1) * sizeof(OffsetNumber);

    // v->spl_left = (OffsetNumber *) palloc(nbytes);
    // left = v->spl_left;
    // v->spl_nleft = 0;

    // v->spl_right = (OffsetNumber *) palloc(nbytes);
    // right = v->spl_right;
    // v->spl_nright = 0;

    // unionL = NULL;
    // unionR = NULL;

    // /* Initialize the raw entry vector. */
    // raw_entryvec = (GISTENTRY **) malloc(entryvec->n * sizeof(void *));
    // for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
        // raw_entryvec[i] = &(entryvec->vector[i]);

    // for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
    // {
        // int         real_index = raw_entryvec[i] - entryvec->vector;

        // tmp_union = DatumGetDataType(entryvec->vector[real_index].key);
        // Assert(tmp_union != NULL);

        // /*
         // * Choose where to put the index entries and update unionL and unionR
         // * accordingly. Append the entries to either v_spl_left or
         // * v_spl_right, and care about the counters.
         // */

        // if (my_choice_is_left(unionL, curl, unionR, curr))
        // {
            // if (unionL == NULL)
                // unionL = tmp_union;
            // else
                // unionL = my_union_implementation(unionL, tmp_union);

            // *left = real_index;
            // ++left;
            // ++(v->spl_nleft);
        // }
        // else
        // {
            // /*
             // * Same on the right
             // */
        // }
    // }

    // v->spl_ldatum = DataTypeGetDatum(unionL);
    // v->spl_rdatum = DataTypeGetDatum(unionR);
    // PG_RETURN_POINTER(v);
// }

// PG_FUNCTION_INFO_V1(my_same);

// Datum
// my_same(PG_FUNCTION_ARGS)
// {
    // prefix_range *v1 = PG_GETARG_PREFIX_RANGE_P(0);
    // prefix_range *v2 = PG_GETARG_PREFIX_RANGE_P(1);
    // bool       *result = (bool *) PG_GETARG_POINTER(2);

    // *result = my_eq(v1, v2);
    // PG_RETURN_POINTER(result);
// }

// PG_FUNCTION_INFO_V1(my_distance);

// Datum
// my_distance(PG_FUNCTION_ARGS)
// {
    // GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
    // data_type  *query = PG_GETARG_DATA_TYPE_P(1);
    // StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
    // /* Oid subtype = PG_GETARG_OID(3); */
    // /* bool *recheck = (bool *) PG_GETARG_POINTER(4); */
    // data_type  *key = DatumGetDataType(entry->key);
    // double      retval;

    // /*
     // * determine return value as a function of strategy, key and query.
     // */

    // PG_RETURN_FLOAT8(retval);
// }

