/*
 * isax.h
 *
 *  Created on: 27Sep.,2016
 *      Author: brynj
 */

#ifndef __ISAX_H__
#define __ISAX_H__

#define MAXSAXBITS 8
#define MAXSAXCARDINALITY 256
#define ISAXWORDLENGTH 14

/*
 * Each iSAX element is a value with an associated cardinality
 *
 */
typedef struct ISAXELEM {
  unsigned char value;
  unsigned char validbits;
} ISAXELEM;

/*
 * An iSAX word is implemented as an fixed length array of iSAX elements.
 * To make it more versatile we'd need to use the ubiquitous varlena
 * structure to allow variable-length words. See pg_trgm for examples of
 * this. (Or could just define storage type isaxelem_ and let PostgreSQL
 * sort it out for you, with potentially some additional overhead).
 *
 */
typedef struct ISAXWORD {
	ISAXELEM elements[ISAXWORDLENGTH];
} ISAXWORD;

void read_isex_elem(char* str, ISAXELEM* result );

#endif /* __ISAX_H__ */
