#include "postgres.h"
#include "access/gist.h"
#include "isax.h"
#include "isax_gist.h"

/**
 * GiST functions to be implemented
 */

PG_FUNCTION_INFO_V1(gist_isax_consistent);

Datum
gist_isax_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	data_type  *query = PG_GETARG_DATA_TYPE_P(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	/* Oid subtype = PG_GETARG_OID(3); */
	bool       *recheck = (bool *) PG_GETARG_POINTER(4);
	data_type  *key = DatumGetDataType(entry->key);
	bool        retval;

	/*
	 * determine return value as a function of strategy, key and query.
	 *
	 * Use GIST_LEAF(entry) to know where you're called in the index tree,
	 * which comes handy when supporting the = operator for example (you could
	 * check for non empty union() in non-leaf nodes and equality in leaf
	 * nodes).
	 */

	*recheck = true;        /* or false if check is exact */

	PG_RETURN_BOOL(retval);
}

PG_FUNCTION_INFO_V1(gist_isax_union);

Datum
gist_isax_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GISTENTRY  *ent = entryvec->vector;
	data_type  *out,
	*tmp,
	*old;
	int         numranges,
	i = 0;

	numranges = entryvec->n;
	tmp = DatumGetDataType(ent[0].key);
	out = tmp;

	if (numranges == 1)
	{
		out = data_type_deep_copy(tmp);

		PG_RETURN_DATA_TYPE_P(out);
	}

	for (i = 1; i < numranges; i++)
	{
		old = out;
		tmp = DatumGetDataType(ent[i].key);
		out = gist_isax_union_implementation(out, tmp);
	}

	PG_RETURN_DATA_TYPE_P(out);
}

PG_FUNCTION_INFO_V1(gist_isax_compress);

Datum
gist_isax_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *retval;

	if (entry->leafkey)
	{
		/* replace entry->key with a compressed version */
		compressed_data_type *compressed_data = palloc(sizeof(compressed_data_type));

		/* fill *compressed_data from entry->key ... */

		retval = palloc(sizeof(GISTENTRY));
		gistentryinit(*retval, PointerGetDatum(compressed_data),
				entry->rel, entry->page, entry->offset, FALSE);
	}
	else
	{
		/* typically we needn't do anything with non-leaf entries */
		retval = entry;
	}

	PG_RETURN_POINTER(retval);
}

PG_FUNCTION_INFO_V1(gist_isax_decompress);

Datum
gist_isax_decompress(PG_FUNCTION_ARGS)
{
	PG_RETURN_POINTER(PG_GETARG_POINTER(0));
}

PG_FUNCTION_INFO_V1(gist_isax_penalty);

Datum
gist_isax_penalty(PG_FUNCTION_ARGS)
{
	GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
	float      *penalty = (float *) PG_GETARG_POINTER(2);
	data_type  *orig = DatumGetDataType(origentry->key);
	data_type  *new = DatumGetDataType(newentry->key);

	*penalty = gist_isax_penalty_implementation(orig, new);
	PG_RETURN_POINTER(penalty);
}

PG_FUNCTION_INFO_V1(gist_isax_picksplit);

Datum
gist_isax_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	OffsetNumber maxoff = entryvec->n - 1;
	GISTENTRY  *ent = entryvec->vector;
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	int         i,
	nbytes;
	OffsetNumber *left,
	*right;
	data_type  *tmp_union;
	data_type  *unionL;
	data_type  *unionR;
	GISTENTRY **raw_entryvec;

	maxoff = entryvec->n - 1;
	nbytes = (maxoff + 1) * sizeof(OffsetNumber);

	v->spl_left = (OffsetNumber *) palloc(nbytes);
	left = v->spl_left;
	v->spl_nleft = 0;

	v->spl_right = (OffsetNumber *) palloc(nbytes);
	right = v->spl_right;
	v->spl_nright = 0;

	unionL = NULL;
	unionR = NULL;

	/* Initialize the raw entry vector. */
	raw_entryvec = (GISTENTRY **) malloc(entryvec->n * sizeof(void *));
	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
		raw_entryvec[i] = &(entryvec->vector[i]);

	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		int         real_index = raw_entryvec[i] - entryvec->vector;

		tmp_union = DatumGetDataType(entryvec->vector[real_index].key);
		Assert(tmp_union != NULL);

		/*
		 * Choose where to put the index entries and update unionL and unionR
		 * accordingly. Append the entries to either v_spl_left or
		 * v_spl_right, and care about the counters.
		 */

		if (gist_isax_choice_is_left(unionL, curl, unionR, curr))
		{
			if (unionL == NULL)
				unionL = tmp_union;
			else
				unionL = gist_isax_union_implementation(unionL, tmp_union);

			*left = real_index;
			++left;
			++(v->spl_nleft);
		}
		else
		{
			/*
			 * Same on the right
			 */
		}
	}

	v->spl_ldatum = DataTypeGetDatum(unionL);
	v->spl_rdatum = DataTypeGetDatum(unionR);
	PG_RETURN_POINTER(v);
}

PG_FUNCTION_INFO_V1(gist_isax_same);

Datum
gist_isax_same(PG_FUNCTION_ARGS)
{
	prefix_range *v1 = PG_GETARG_PREFIX_RANGE_P(0);
	prefix_range *v2 = PG_GETARG_PREFIX_RANGE_P(1);
	bool       *result = (bool *) PG_GETARG_POINTER(2);

	*result = gist_isax_eq(v1, v2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(gist_isax_distance);

Datum
gist_isax_distance(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	data_type  *query = PG_GETARG_DATA_TYPE_P(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	/* Oid subtype = PG_GETARG_OID(3); */
	/* bool *recheck = (bool *) PG_GETARG_POINTER(4); */
	data_type  *key = DatumGetDataType(entry->key);
	double      retval;

	/*
	 * determine return value as a function of strategy, key and query.
	 */

	PG_RETURN_FLOAT8(retval);
}
