/*
 * isax.h
 *
 *  Created on: 27Sep.,2016
 *      Author: brynj
 *  Using idioms from trgm.h in contrib/pg_trgm
 */

#ifndef __ISAX_H__
#define __ISAX_H__

/*
 * Each iSAX element is a value with an associated cardinality
 */
typedef struct ISAXELEM {
  unsigned char lower;
  unsigned char cardinality;
} ISAXELEM;

/*
 * An iSAX word is based upon the ubiquitous varlena structure.
 * This could allow variable-length words.
 */
typedef struct ISAXWORD
{
  int32      vl_len_;
  char   data[FLEXIBLE_ARRAY_MEMBER];
} ISAXWORD;

#define ISAXHDRSIZE	(VARHDRSZ)
#define GETARR(x)		( (ISAXELEM*)( (char*)x+ISAXHDRSIZE ) )
#define ARRNELEM(x) ( ( VARSIZE(x) - ISAXHDRSIZE )/sizeof(ISAXELEM) )

#endif /* __ISAX_H__ */
