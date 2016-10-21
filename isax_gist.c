#include "postgres.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "access/gist.h"
#include "access/stratnum.h"
#include "isax.h"
#include "isax_gist.h"
#include "fmgr.h"


/**
 * GiST functions to be implemented
 */

 #define ARRPTR(x)  ( (float4 *) ARR_DATA_PTR(x) );

double isax_gist_distance_threshold = 0.5f;
static float saxbp[] ={  -2.66006747, -2.41755902, -2.26622681, -2.15387469, -2.06352790, -1.98742789, -1.92135077, -1.86273187, -1.80989224, -1.76167041, -1.71722812, -1.67593972, -1.63732538, -1.60100866, -1.56668859, -1.53412054, -1.50310294, -1.47346758, -1.44507258, -1.41779714, -1.39153749, -1.36620382, -1.34171784, -1.31801090, -1.29502241, -1.27269864, -1.25099172, -1.22985876, -1.20926123, -1.18916435, -1.16953661, -1.15034938, -1.13157656, -1.11319428, -1.09518065, -1.07751557, -1.06018048, -1.04315826, -1.02643306, -1.00999017, -0.99381591, -0.97789754, -0.96222320, -0.94678176, -0.93156283, -0.91655667, -0.90175411, -0.88714656, -0.87272589, -0.85848447, -0.84441508, -0.83051088, -0.81676542, -0.80317257, -0.78972652, -0.77642176, -0.76325304, -0.75021538, -0.73730400, -0.72451438, -0.71184220, -0.69928330, -0.68683375, -0.67448975, -0.66224768, -0.65010407, -0.63805558, -0.62609901, -0.61423129, -0.60244945, -0.59075066, -0.57913216, -0.56759132, -0.55612559, -0.54473251, -0.53340971, -0.52215488, -0.51096581, -0.49984034, -0.48877641, -0.47777199, -0.46682512, -0.45593392, -0.44509652, -0.43431116, -0.42357608, -0.41288960, -0.40225007, -0.39165587, -0.38110545, -0.37059729, -0.36012989, -0.34970180, -0.33931161, -0.32895791, -0.31863936, -0.30835463, -0.29810241, -0.28788143, -0.27769044, -0.26752821, -0.25739353, -0.24728522, -0.23720211, -0.22714306, -0.21710695, -0.20709265, -0.19709908, -0.18712516, -0.17716982, -0.16723201, -0.15731068, -0.14740482, -0.13751340, -0.12763542, -0.11776987, -0.10791578, -0.09807215, -0.08823802, -0.07841241, -0.06859437, -0.05878294, -0.04897716, -0.03917609, -0.02937878, -0.01958429, -0.00979167,  0.00000000,  0.00979167,  0.01958429,  0.02937878,  0.03917609,  0.04897716,  0.05878294,  0.06859437,  0.07841241,  0.08823802,  0.09807215,  0.10791578,  0.11776987,  0.12763542,  0.13751340,  0.14740482,  0.15731068,  0.16723201,  0.17716982,  0.18712516,  0.19709908,  0.20709265,  0.21710695,  0.22714306,  0.23720211,  0.24728522,  0.25739353,  0.26752821,  0.27769044,  0.28788143,  0.29810241,  0.30835463,  0.31863936,  0.32895791,  0.33931161,  0.34970180,  0.36012989,  0.37059729,  0.38110545,  0.39165587,  0.40225007,  0.41288960,  0.42357608,  0.43431116,  0.44509652,  0.45593392,  0.46682512,  0.47777199,  0.48877641,  0.49984034,  0.51096581,  0.52215488,  0.53340971,  0.54473251,  0.55612559,  0.56759132,  0.57913216,  0.59075066,  0.60244945,  0.61423129,  0.62609901,  0.63805558,  0.65010407,  0.66224768,  0.67448975,  0.68683375,  0.69928330,  0.71184220,  0.72451438,  0.73730400,  0.75021538,  0.76325304,  0.77642176,  0.78972652,  0.80317257,  0.81676542,  0.83051088,  0.84441508,  0.85848447,  0.87272589,  0.88714656,  0.90175411,  0.91655667,  0.93156283,  0.94678176,  0.96222320,  0.97789754,  0.99381591,  1.00999017,  1.02643306,  1.04315826,  1.06018048,  1.07751557,  1.09518065,  1.11319428,  1.13157656,  1.15034938,  1.16953661,  1.18916435,  1.20926123,  1.22985876,  1.25099172,  1.27269864,  1.29502241,  1.31801090,  1.34171784,  1.36620382,  1.39153749,  1.41779714,  1.44507258,  1.47346758,  1.50310294,  1.53412054,  1.56668859,  1.60100866,  1.63732538,  1.67593972,  1.71722812,  1.76167041,  1.80989224,  1.86273187,  1.92135077,  1.98742789,  2.06352790,  2.15387469,  2.26622681,  2.41755902,  2.66006747, 100.0};

float4* gist_isax_ts_to_paa(ArrayType* key);
ISAXWORD* gist_isax_paa_to_isax(float4* paa);
double gist_isax_mindist_paa_isax(ISAXWORD* entry, ArrayType* key);
ISAXWORD* gist_isax_union_implementation(ISAXWORD* left,ISAXWORD* right);
float gist_isax_penalty_implementation(ISAXWORD* orig, ISAXWORD* new );
float gist_isax_calc_lower_bp(int v, int card);
float gist_isax_calc_upper_bp(int v, int card);

PG_FUNCTION_INFO_V1(gist_isax_consistent);

Datum
gist_isax_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	ArrayType  *query = PG_GETARG_ARRAYTYPE_P(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	/* Oid subtype = PG_GETARG_OID(3); */
	bool       *recheck = (bool *) PG_GETARG_POINTER(4);
	ISAXWORD  *key = (ISAXWORD*)DatumGetPointer(entry->key);
	bool        retval;

	/*
	 * determine return value as a function of strategy, key and query.
	 *
	 * Use GIST_LEAF(entry) to know where you're called in the index tree,
	 * which comes handy when supporting the = operator for example (you could
	 * check for non empty union() in non-leaf nodes and equality in leaf
	 * nodes).
	 */
	 if(strategy != SimilarityStrategyNumber){
		 retval = false;
	 }
	 else{
		 if(gist_isax_mindist_paa_isax(key, query) <= isax_gist_distance_threshold){
			 retval = true;
		 }
		 else {
			 retval = false;
		 }
	 }

	*recheck = true;        /* or false if check is exact */


	PG_RETURN_BOOL(retval);
}

PG_FUNCTION_INFO_V1(gist_isax_union);

Datum
gist_isax_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GISTENTRY  *ent = entryvec->vector;
	ISAXWORD  *out,
						*tmp;
	int numranges,
			i = 0;

	numranges = entryvec->n;
	tmp = (ISAXWORD*) DatumGetPointer(ent[0].key);
	out = tmp;

	if (numranges == 1)
	{
		PG_RETURN_POINTER(out);
	}

	for (i = 1; i < numranges; i++)
	{
		tmp = (ISAXWORD*) DatumGetPointer(ent[i].key);
		out = gist_isax_union_implementation(out, tmp);
	}

	PG_RETURN_POINTER(out);
}

PG_FUNCTION_INFO_V1(gist_isax_compress);

Datum
gist_isax_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *retval;
	ArrayType  *key = (ArrayType*)DatumGetPointer(entry->key);
	float4* paa;

	if (entry->leafkey)
	{

		/* replace entry->key with a compressed version */
		ISAXWORD *compressed_data = palloc(sizeof(ISAXWORD));

		/* fill *compressed_data from entry->key ... */
		paa = gist_isax_ts_to_paa(key);
		compressed_data = gist_isax_paa_to_isax(paa);

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
	ISAXWORD  *orig = (ISAXWORD*) DatumGetPointer(origentry->key);
	ISAXWORD  *new = (ISAXWORD*) DatumGetPointer(newentry->key);

	*penalty = gist_isax_penalty_implementation(orig, new);
	PG_RETURN_POINTER(penalty);
}

PG_FUNCTION_INFO_V1(gist_isax_picksplit);

Datum
gist_isax_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	OffsetNumber maxoff = entryvec->n - 1;
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	int i,
	 		w =14,
			nbytes;
	OffsetNumber *left,
							 *right;
	ISAXWORD  *tmp_union;
	ISAXWORD  *unionL;
	ISAXWORD  *unionR;
	GISTENTRY **raw_entryvec;
	int c_plus[w],
	 		bp_plus[w],
	 		bp_minus[w];
	int p = 0,
			bp_delta = 0,
			delta_max;
	float	t;

	//Initialize c+, b+, b-
 	for (i = 0; i < w; i++){
		c_plus[i] = 256;
		bp_plus[i] = 0;
		bp_minus[i] = 255;
	}

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

	//This first loop is to assign c+, b+, b-
	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		int real_index = raw_entryvec[i] - entryvec->vector,
				j;

		tmp_union = (ISAXWORD*) DatumGetPointer(entryvec->vector[real_index].key);
		Assert(tmp_union != NULL);

		/*
		 * Choose where to put the index entries and update unionL and unionR
		 * accordingly. Append the entries to either v_spl_left or
		 * v_spl_right, and care about the counters.
		 */

		 for(j = 0; j < w; j+=1){
			 int c = 1 << ((int) tmp_union->elements[j].validbits);
			 int bp = 		(int) tmp_union->elements[j].value;
			 int tmp_c = c_plus[i]; //Current cardinality of b+ and b-
			 int c_delta = 0;

			 c_plus[j] = c_plus[j] < c ? c_plus[j] : c; //min(c_plus, c)

			 //Reduce cardinality from c_plus to c
			 c_delta = tmp_c > c ? (tmp_c - c) : 0;
			 bp_plus[j] = bp_plus[j] >> c_delta;
			 bp_minus[j] = bp_minus[j] >> c_delta;

			 bp_plus[j] = bp_plus[j] > bp ? bp_plus[j] : bp; //max(bp_plus, bp)
			 bp_minus[j] = bp_minus[j] < bp ? bp_minus[j] : bp; //min(bp_plus, bp)
		 }
	}

	//Getting t (the split breakpoint)
	delta_max = 0;
	for(i = 0; i < w; i+=1){
		bp_delta = bp_plus[i] - bp_minus[i];
		if (bp_delta > delta_max){
			delta_max = bp_delta;
			p = i;
		}
	}
	t = ((float)bp_plus[p] + (float)bp_minus[p])/2;

	//This second loop is to choose which side to split
	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		int real_index = raw_entryvec[i] - entryvec->vector,
				j;

		//TODO: Is tmp_union e in E  or iSAX key of e ?
		//Picksplit: go through all values and decide where to split
		tmp_union = (ISAXWORD*) DatumGetPointer(entryvec->vector[real_index].key);
		Assert(tmp_union != NULL);

		for(j = 0; j < w; j += 1){
			if (tmp_union->elements[i].value <= t)
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
				if (unionR == NULL)
					unionR = tmp_union;
				else
					unionR = gist_isax_union_implementation(unionR, tmp_union);

				*right = real_index;
				++right;
				++(v->spl_nright);
			}
		}
	}

	v->spl_ldatum = PointerGetDatum(unionL);
	v->spl_rdatum = PointerGetDatum(unionR);
	PG_RETURN_POINTER(v);
}

//TODO: do I need to change same and distance ? Answer: no
PG_FUNCTION_INFO_V1(gist_isax_same);

Datum
gist_isax_same(PG_FUNCTION_ARGS)
{
	int i,
			w = 14;
	ISAXWORD *v1 = (ISAXWORD*) PG_GETARG_POINTER(0);
	ISAXWORD *v2 = (ISAXWORD*) PG_GETARG_POINTER(1);
	bool       *result = (bool *) PG_GETARG_POINTER(2);

	*result = true;
	for (i=0; i < w; i+=1){
		int value1 = (int)v1->elements[i].value;
		int validbits1 = (int)v1->elements[i].validbits;
		int value2 = (int)v2->elements[i].value;
		int validbits2 = (int)v2->elements[i].validbits;

		if(value1 != value2 || validbits1 != validbits2){
			*result = false;
		}
	}

	PG_RETURN_POINTER(result);
}

//Dont need to declare
// PG_FUNCTION_INFO_V1(gist_isax_distance);
//
// Datum
// gist_isax_distance(PG_FUNCTION_ARGS)
// {
// 	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
// 	data_type  *query = PG_GETARG_DATA_TYPE_P(1);
// 	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
// 	/* Oid subtype = PG_GETARG_OID(3); */
// 	/* bool *recheck = (bool *) PG_GETARG_POINTER(4); */
// 	data_type  *key = (ISAXWORD*) DatumGetPointer(entry->key);
// 	double      retval;
//
// 	/*
// 	 * determine return value as a function of strategy, key and query.
// 	 */
//
// 	PG_RETURN_FLOAT8(retval);
// }

//START
float4*
gist_isax_ts_to_paa(ArrayType* key){
	int* dims;
  int i, j,
      ndims,
      n,
      w = 14;
  ArrayType* ts = key;
	float4* result = (float4*) palloc(w*sizeof(float4));

  float4* val ; //Array pointer for ts
	float4* val_res = result; //Array pointer for result

  ndims = ARR_NDIM(ts);
  dims = ARR_DIMS(ts);
  n = ArrayGetNItems(ndims,dims);

  val = ARRPTR(ts);
  for(i = 1 ; i <= w; i+=1){
    float4 sum = 0;
    for(j = n/w * (i-1)+1; j <= n/w * i; j+=1){
      sum += *val;
      val++;
    }
    *val_res = (float)w/(float)n * sum;
		val_res++;
  }

	return result;
}

ISAXWORD*
gist_isax_paa_to_isax(float4* paa){
	/*Turning PAA representation to isax words in format of v:card*/
	int i = 0, j = 0,
			w = 14,
			v = 0,
			card = 256,
			n = 8;
	ISAXWORD* isax = (ISAXWORD*) palloc(sizeof(ISAXWORD));

	for(i = 0 ; i < w; i += 1){
		//Bottom breakpoint
		if(*paa<saxbp[0]){
			v = 0;
		}
		else{
			for(j = 1; j < card-1; j += 1){
				if (*paa >= saxbp[j-1] && *paa < saxbp[j]){
					v = j;
					break;
				}
			}
		}

		isax->elements[i].value = (unsigned char)(v);
		isax->elements[i].validbits = (unsigned char)(n);

		paa++;
	}
	return(isax);
}

double
gist_isax_mindist_paa_isax(ISAXWORD* entry, ArrayType* key){
	double mindist = 0;
	int i = 0,
			n = 140,
			w = 14;
	float4* tpaa = gist_isax_ts_to_paa(key);

	for(i = 0; i < w; i += 1){
		int v = (int) entry->elements[i].value,
				card =  1 << ((int) entry->elements[i].validbits);
		float beta_L = gist_isax_calc_lower_bp(v,card),
					beta_U = gist_isax_calc_upper_bp(v,card),
					delta;

		if(*tpaa < beta_L){
			delta = beta_L - *tpaa;
		}
		else if (*tpaa > beta_U){
			delta = *tpaa - beta_U;
		}
		else{
			delta = 0;
		}
		mindist += delta * delta;

		tpaa++;
	}

	mindist = sqrt((float)n/(float)w) * mindist;
	return(mindist);
}

ISAXWORD*
gist_isax_union_implementation(ISAXWORD* left,ISAXWORD* right){
	int i,
      w = 14;
  int bp_left[w],
      bp_right[w],
      n_left[w],
      n_right[w];
  ISAXWORD* result = (ISAXWORD*) palloc(sizeof(ISAXWORD));

  //Initializing
  for(i =0; i < w; i+=1){
    bp_left[i] = (int)left->elements[i].value;
    n_left[i] = (int)left->elements[i].validbits;
    bp_right[i] = (int)right->elements[i].value;
    n_right[i] = (int)right->elements[i].validbits;
  }

  //Getting result
  for (i = 0; i < w; i+=1){
  	int tmp_left = bp_left[i],
        tmp_right = bp_right[i],
        union_val,
        union_bits = 0;

    //Equivalating validbits
    if(n_right[i] > n_left[i]){
      tmp_right = tmp_right >> (n_right[i] - n_left[i]);
    }
    else if (n_right[i] < n_left[i]){
      tmp_left = tmp_left >> (n_left[i] - n_right[i]);
    }

    while(tmp_left != tmp_right){
      tmp_left = tmp_left >> 1;
      tmp_right = tmp_right >> 1;
    }

    union_val = tmp_left;

    while(tmp_left > 0){
      tmp_left = tmp_left >> 1;
      union_bits++;
    }

  	result->elements[i].value = (unsigned char) union_val;
    result->elements[i].validbits = (unsigned char)union_bits;
  }
	return(result);
}

float
gist_isax_penalty_implementation(ISAXWORD* orig, ISAXWORD* new ){

	float delta = 0;
	ISAXELEM* e_A = orig->elements;
	ISAXELEM* e_B = new->elements;
	int i,
			w = 14;

	for (i = 0; i < w; i += 1){
		int c_A, c_B;
		c_A = 1 << ((int)e_A[i].validbits) ;
		c_B = 1 << ((int)e_B[i].validbits) ;
		delta += (c_B - c_A);
	}

	return(delta);
}



float
gist_isax_calc_lower_bp(int v, int card){
	int mult = 256/card;
	float bp;

	if(v == 0){
		bp = -100;
	}
	else{
		bp = saxbp[(v)*mult -1];
	}

	return(bp);
}

float
gist_isax_calc_upper_bp(int v, int card){
	//Assume that card is a demoninator of 256
	int mult = 256/card;
	float bp;

	bp = saxbp[(v+1)*mult -1];

	return(bp);
}
