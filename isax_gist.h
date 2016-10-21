#ifndef __ISAX_GIST_H__
#define __ISAX_GIST_H__

#define SimilarityStrategyNumber 1

float gist_isax_calc_upper_bp(int v, int card);
float gist_isax_calc_lower_bp(int v, int card);
float4* gist_isax_ts_to_paa(ArrayType* key);
float gist_isax_penalty_implementation(ISAXWORD* orig, ISAXWORD* new );
ISAXWORD* gist_isax_union_implementation(ISAXWORD* left,ISAXWORD* right);
double gist_isax_mindist_paa_isax(ISAXWORD* entry, ArrayType* key);
ISAXWORD* gist_isax_paa_to_isax(float4* paa);

#endif /* __ISAX_GIST_H__ */
