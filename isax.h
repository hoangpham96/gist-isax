/*
 * isax.h
 *
 *  Created on: 27Sep.,2016
 *      Author: brynj
 *  Using idioms from trgm.h in contrib/pg_trgm
 */

#ifndef __ISAX_H__
#define __ISAX_H__

#define MAXSAXBITS 8
#define MAXSAXCARDINALITY 256

/*
 * Each iSAX element is a value with an associated cardinality
 *
 * An iSAX word is implemented as an array of iSAX elements, using the
 * internal array representation (the ubiquitous varlena structure).
 * This could allow variable-length words.
 */
typedef struct ISAXELEM {
  unsigned char value;
  unsigned char validbits;
} ISAXELEM;

#endif /* __ISAX_H__ */
