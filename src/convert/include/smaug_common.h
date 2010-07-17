#ifndef SMAUG_COMMON_H
#define SMAUG_COMMON_H

#define TRUE 1
#define FALSE 0
#define LOWER(c)              ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define IS_SET(flag, bit)     ((flag) & (bit))

typedef int				sh_int;
typedef int				bool;
typedef struct	extended_bitvector	EXT_BV;

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM		31	/* extended bitmask   ( INTBITS - 1 )	*/
#define RSV		5	/* right-shift value  ( sqrt(XBM+1) )	*/
#define XBI		4	/* integers in an extended bitvector	*/
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
    unsigned int		bits[XBI]; /* Needs to be unsigned to compile in Redhat 6 - Samson */
};

/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool ext_is_empty( EXT_BV *bits );
void ext_clear_bits( EXT_BV *bits );
int ext_has_bits( EXT_BV *var, EXT_BV *bits );
bool ext_same_bits( EXT_BV *var, EXT_BV *bits );
void ext_set_bits( EXT_BV *var, EXT_BV *bits );
void ext_remove_bits( EXT_BV *var, EXT_BV *bits );
void ext_toggle_bits( EXT_BV *var, EXT_BV *bits );

/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/* and boring old ones */
#define IS_SET(flag, bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

#define HAS_SPELL_INDEX -1

#endif
