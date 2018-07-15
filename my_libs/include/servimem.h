/*****************************************************************************
  Purpose:     Auxiliary functions working on the heap/stack only.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: SERVIMEM.H,v 1.3 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INCL_SERVIMEM
#define INCL_SERVIMEM

#include <string.h>	/* size_t, ... */
#include "portable.h"

size_t slist_len(const char *slist) ;
	/* returns length of slist without last \0, just like strlen() */
	/* Examples: slist_len(NULL)   = 0          */
	/*           slist_len("\0")   = 1,         */
	/*           slist_len("bla\0hello\0") = 10 */
	
char *slist_dup(const char *slist) ;
	/* returns allocated duplicate slist, or NULL in bad case */

char *slist_str(const char *slist, const char *str) ;
	/* searches slist for first occurence of specified string */
	/* like strstr(), but str must match a full element !     */
	/* Examples: slist_str("bla\0hello\0", "bla") matches     */
	/*           slist_str("bla\0hello\0", "bl" ) matches not */



/********************************************************************
*/
	UINT16 swap_int16(UINT16 in) ;
	UINT32 swap_int32(UINT32 in) ;
/*
* Swaps high and low byte of a 16 bit integer, resp.
* swaps inverts byte sequence of a 32 bit integer.
* Application: Motorola <=> Intel format conversions. 
* 
* RETURN:  integer with changed byte order 
* 
********************************************************************/

/********************************************************************
*/
	UINT32 read_int32(const UBYTE *address) ;
/*
* needed in order to read a long integer
* from an odd address. This normally will lead to an address error. 
* The function "read_long" manages this problem by reading 
* byte by byte.
* 
* RETURN:  fixed length number read 
* 
********************************************************************/

/********************************************************************
*/ 
	UINT16 read_int16(const UBYTE *address) ;
/*
* needed in order to read an integer
* from an odd address. This normally will lead to an address error. 
* The function "read_int" manages this problem by reading 
* byte by byte.
* 
* RETURN:  fixed length number read 
* 
********************************************************************/

/********************************************************************
*/
unsigned int read_var_len(
	const UBYTE *address, UINT32 *P_read_number, UBYTE *P_read_bytes) ;
/*
* Read a variable length number
* as described in the MIDI File Specification. 
* 
* INPUT:   address:  RAM address where the variable length number
*                    has to be read from
* OUTPUT:  *P_read_number:  variable length number read as 
*                             unsigned long integer
*          *P_read_bytes:  length of variable length number 
* RETURN:  0 = good, -1 = error 
* 
********************************************************************/

#endif /* include blocker */
