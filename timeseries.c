#include "postgres.h"
/* Array handling functions */
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "fmgr.h"
#include "timeseries.h"
//TODO: why are the functions not included ?
#include "isax.h"
#include "isax_gist.h"
#include <float.h>
#include <math.h>
#include <string.h>
/*
 * The functions here are written to conform with the Postgres function manager and
 * function-call interface. See http://doxygen.postgresql.org/fmgr_8h_source.html
 */

char* write_isax(ISAXWORD* input);
ISAXWORD* read_isax(char* input);

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
static float saxbp[] ={  -2.66006747, -2.41755902, -2.26622681, -2.15387469, -2.06352790, -1.98742789, -1.92135077, -1.86273187, -1.80989224, -1.76167041, -1.71722812, -1.67593972, -1.63732538, -1.60100866, -1.56668859, -1.53412054, -1.50310294, -1.47346758, -1.44507258, -1.41779714, -1.39153749, -1.36620382, -1.34171784, -1.31801090, -1.29502241, -1.27269864, -1.25099172, -1.22985876, -1.20926123, -1.18916435, -1.16953661, -1.15034938, -1.13157656, -1.11319428, -1.09518065, -1.07751557, -1.06018048, -1.04315826, -1.02643306, -1.00999017, -0.99381591, -0.97789754, -0.96222320, -0.94678176, -0.93156283, -0.91655667, -0.90175411, -0.88714656, -0.87272589, -0.85848447, -0.84441508, -0.83051088, -0.81676542, -0.80317257, -0.78972652, -0.77642176, -0.76325304, -0.75021538, -0.73730400, -0.72451438, -0.71184220, -0.69928330, -0.68683375, -0.67448975, -0.66224768, -0.65010407, -0.63805558, -0.62609901, -0.61423129, -0.60244945, -0.59075066, -0.57913216, -0.56759132, -0.55612559, -0.54473251, -0.53340971, -0.52215488, -0.51096581, -0.49984034, -0.48877641, -0.47777199, -0.46682512, -0.45593392, -0.44509652, -0.43431116, -0.42357608, -0.41288960, -0.40225007, -0.39165587, -0.38110545, -0.37059729, -0.36012989, -0.34970180, -0.33931161, -0.32895791, -0.31863936, -0.30835463, -0.29810241, -0.28788143, -0.27769044, -0.26752821, -0.25739353, -0.24728522, -0.23720211, -0.22714306, -0.21710695, -0.20709265, -0.19709908, -0.18712516, -0.17716982, -0.16723201, -0.15731068, -0.14740482, -0.13751340, -0.12763542, -0.11776987, -0.10791578, -0.09807215, -0.08823802, -0.07841241, -0.06859437, -0.05878294, -0.04897716, -0.03917609, -0.02937878, -0.01958429, -0.00979167,  0.00000000,  0.00979167,  0.01958429,  0.02937878,  0.03917609,  0.04897716,  0.05878294,  0.06859437,  0.07841241,  0.08823802,  0.09807215,  0.10791578,  0.11776987,  0.12763542,  0.13751340,  0.14740482,  0.15731068,  0.16723201,  0.17716982,  0.18712516,  0.19709908,  0.20709265,  0.21710695,  0.22714306,  0.23720211,  0.24728522,  0.25739353,  0.26752821,  0.27769044,  0.28788143,  0.29810241,  0.30835463,  0.31863936,  0.32895791,  0.33931161,  0.34970180,  0.36012989,  0.37059729,  0.38110545,  0.39165587,  0.40225007,  0.41288960,  0.42357608,  0.43431116,  0.44509652,  0.45593392,  0.46682512,  0.47777199,  0.48877641,  0.49984034,  0.51096581,  0.52215488,  0.53340971,  0.54473251,  0.55612559,  0.56759132,  0.57913216,  0.59075066,  0.60244945,  0.61423129,  0.62609901,  0.63805558,  0.65010407,  0.66224768,  0.67448975,  0.68683375,  0.69928330,  0.71184220,  0.72451438,  0.73730400,  0.75021538,  0.76325304,  0.77642176,  0.78972652,  0.80317257,  0.81676542,  0.83051088,  0.84441508,  0.85848447,  0.87272589,  0.88714656,  0.90175411,  0.91655667,  0.93156283,  0.94678176,  0.96222320,  0.97789754,  0.99381591,  1.00999017,  1.02643306,  1.04315826,  1.06018048,  1.07751557,  1.09518065,  1.11319428,  1.13157656,  1.15034938,  1.16953661,  1.18916435,  1.20926123,  1.22985876,  1.25099172,  1.27269864,  1.29502241,  1.31801090,  1.34171784,  1.36620382,  1.39153749,  1.41779714,  1.44507258,  1.47346758,  1.50310294,  1.53412054,  1.56668859,  1.60100866,  1.63732538,  1.67593972,  1.71722812,  1.76167041,  1.80989224,  1.86273187,  1.92135077,  1.98742789,  2.06352790,  2.15387469,  2.26622681,  2.41755902,  2.66006747, 100.0};


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

/*
* All of the test functions go here since I'm too lazy to create a new file and have to set up git and stuff.
*/
PG_FUNCTION_INFO_V1(ts_to_paa);
Datum ts_to_paa(PG_FUNCTION_ARGS);

Datum
ts_to_paa(PG_FUNCTION_ARGS)
{
  int* dims;
  int i, j,
      ndims,
      n,
      w = 14;
  ArrayType* ts = PG_GETARG_ARRAYTYPE_P(0);
  ArrayType* result;

  Datum* c = (Datum *) palloc(w*sizeof(Datum));
  float4* val; //Array pointer for ts

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
    c[i-1] = Float4GetDatum((float)w/(float)n * sum);
  }

  //Putting data into result
  result = construct_array(c,w,FLOAT4OID, sizeof(float), true, 'i');

	PG_RETURN_ARRAYTYPE_P(result);
  pfree(c);
}

PG_FUNCTION_INFO_V1(paa_to_isax);
Datum paa_to_isax(PG_FUNCTION_ARGS);

Datum
paa_to_isax(PG_FUNCTION_ARGS)
{
  int i, j,
      w = 14,
      v = 0,
      card = 256,
      n = 8;
  float4* val;
  ArrayType* paa = (ArrayType*) PG_GETARG_ARRAYTYPE_P(0);
  ISAXWORD* isax = (ISAXWORD*) palloc(sizeof(ISAXWORD));

  char* result;


  val = ARRPTR(paa); //Array pointer for paa
  for(i = 0 ; i < w; i += 1){
    //Bottom breakpoint
    if(*val < saxbp[0]){
    	v = 0;
    }
    else{
    	for(j = 1; j<card-1; j += 1){
    		if (*val >= saxbp[j-1] && *val < saxbp[j]){
    			v = j;
    			break;
    		}
    	}
    }

    isax->elements[i].value = (unsigned char) v;
    isax->elements[i].validbits = (unsigned char) n;

    val++;
  }

  result = write_isax(isax);
  PG_RETURN_CSTRING(result);
  pfree(result);
}

//TODO: finish
// PG_FUNCTION_INFO_V1(mindist_paa_isax);
// Datum mindist_paa_isax(PG_FUNCTION_ARGS);
//
// Datum mindist_paa_isax(PG_FUNCTION_ARGS){
//   float mindist = 0;
// 	int i = 0,
// 			n = 140,
// 			w = 14;
//   char* isaxbuffer = PG_GETARG_CSTRING(0);
//   char* token;
// 	ISAXWORD* isax = (ISAXWORD *) palloc(sizeof(ISAXWORD));
//  ArrayType* key = PG_GETARG_ARRAYTYPE_P(1);
// 	float4* tpaa = gist_isax_ts_to_paa(key);
//
// isax = read_isax(isaxbuffer);
//
//   //Main code
// 	for(i = 1; i <= w; i +=1){
// 		ISAXELEM* isaxelem = &isax->elements[i-1];
// 		int v = ((int) isaxelem->value)+1,
// 				card =  1 << ((int) isaxelem->validbits);
// 		float beta_L = gist_isax_calc_lower_bp(v,card),
// 					beta_U = gist_isax_calc_upper_bp(v,card),
// 					delta;
//
// 		if(*tpaa < beta_L){
// 			delta = beta_L - *tpaa;
// 		}
// 		else if (*tpaa > beta_U){
// 			delta = *tpaa - beta_U;
// 		}
// 		else{
// 			delta = 0;
// 		}
// 		mindist += delta * delta;
//
// 		tpaa++;
// 	}
//
// 	mindist = sqrt((float)n/(float)w) * mindist;
//   PG_RETURN_FLOAT4(mindist);
// }

//TODO: finish
// PG_FUNCTION_INFO_V1(penalty_implementation);
// Datum penalty_implementation(PG_FUNCTION_ARGS);
//
// Datum penalty_implementation(PG_FUNCTION_ARGS){
//   float delta = 0;
//   ISAXWORD* orig = read_isax(PG_GETARG_CSTRING(0));
//   ISAXWORD* NEW = read_isax(PG_GETARG_CSTRING(1));
//  	ISAXELEM* e_A = orig->elements;
//  	ISAXELEM* e_B = new->elements;
//  	int i,
//  		 	w = 14;
//
//  	for (i = 0; i < w; i ){
//  		int c_A, c_B;
//  		c_A = 1 << ((int)e_A[i]->validbits) ;
//  		c_B = 1 << ((int)e_B[i]->validbits) ;
//  		delta += (c_B - c_A);
//  	}
//
//  	PG_RETURN_FLOAT4(delta);
// }

//TODO: finish
// PG_FUNCTION_INFO_V1(union_implementation);
// Datum union_implementation(PG_FUNCTION_ARGS);
//
// Datum union_implementation(PG_FUNCTION_ARGS){
//  int i, j;
//  int bp_plus[w],
// 	bp_minus[w],
//  ISAXWORD* result = palloc(sizeof(ISAXWORD));
//
// //Initializing
// for(i =0; i < w; i+=1){
// 	bp_plus[i] = 0;
// 	bp_minus[i = 255;
// }
//
// //Getting bp's for left
// for (i = 0; i < w; i+=1){
// 	int bp = (int)left->elements[i].value ;
// 	bp_plus[i] = bp_plus[i] > bp ? bp_plus[i] : bp; //max(bp_plus[i], bp)
// 	bp_minus[i] = bp_minus[i] < bp ? bp_minus[i] : bp; //min(bp_minus[i], bp)
// }
//
// //Getting bp's for right
// for (i = 0; i < w; i+=1){
// 	int bp = (int)right->elements[i].value ;
// 	bp_plus[i] = bp_plus[i] > bp ? bp_plus[i] : bp; //max(bp_plus[i], bp)
// 	bp_minus[i] = bp_minus[i] < bp ? bp_minus[i] : bp; //min(bp_minus[i], bp)
// }
//
// //Getting result
// for (i = 0; i < w; i+=1){
// 	int v,
// 			tmp_plus = bp_plus[i],
// 			tmp_minus = bp_minus[i],
// 	 		n,
// 			done = 0;
// 	ISAXELEM* isaxelem;
//
// 	while(done == 0){
// 		if (tmp_plus == tmp_minus){
// 			done = 1;
// 		}
// 		else{
// 			tmp_plus >> 1;
// 			tmp_minus >> 1
// 		}
// 	}
//
// 	while(tmp_plus != 0){
// 		tmp_plus>>1;
// 		++n;
// 	}
// 	//TODO: what is v ? Answer: common bin
// 	isaxelem->value = (unsigned char)v;
// 	isaxelem->validbits = (unsigned char)n;
// 	result->elements[i] = isaxelem;
// }
// PG_RETURN_CSTRING(result);
// }

PG_FUNCTION_INFO_V1(calc_lower_bp);
Datum calc_lower_bp(PG_FUNCTION_ARGS);

Datum
calc_lower_bp(PG_FUNCTION_ARGS){
  int16 v = PG_GETARG_UINT16(0);
  int16 card = PG_GETARG_UINT16(1);
  int mult = 256/card;
  float bp;

  if(v == 1){
    bp = -100;
  }
  else{
    bp = saxbp[(v-1)*mult -1];
  }

  PG_RETURN_FLOAT4(bp);
}

PG_FUNCTION_INFO_V1(calc_upper_bp);
Datum calc_upper_bp(PG_FUNCTION_ARGS);

Datum
calc_upper_bp(PG_FUNCTION_ARGS){
  int16 v = PG_GETARG_UINT16(0);
  int16 card = PG_GETARG_UINT16(1);
  int mult = 256/card;
	float bp;

	bp = saxbp[v*mult -1];

  PG_RETURN_FLOAT4(bp);
}

// TODO: finish
ISAXWORD*
read_isax(char* input){
  ISAXWORD* result = (ISAXWORD*) palloc(sizeof(ISAXWORD));
  int i = 0,
      len;
  char *str, *str2,
       *pch;
  char* token;
  const char s[2] = ",";

  str = input;

  pch =  strchr(str,'}');
  len = pch - str - 1;
  str2 = palloc(len+1);
  strncpy(str2,str+1,len);
  *(str2+len) = '\0';
  pch = str2;

  while (( token = strsep(&pch, s) )){
    ISAXELEM* isaxelem = (ISAXELEM*) palloc(sizeof(ISAXELEM));

    read_isex_elem(token, isaxelem);
    result->elements[i] = *isaxelem;
    i++;
  }

  return(result);
}

char*
write_isax(ISAXWORD* input){
  int i,
      v,
      card,
      w = 14,
      l = 2 + w*8;
  char* buffer;
  char* result = (char*) palloc(l*sizeof(char));

  buffer = result;
  *buffer = '{';

  for(i = 0; i < w; i+=1){
    v = (int)input->elements[i].value;
    card = 1 << ((int)input->elements[i].validbits);
    ++buffer;
    buffer += sprintf(buffer, "%d:%d", v,card);
    *buffer = ',';
  }
  *buffer = '}';
  *(++buffer) = '\0';
  return(result);
}
