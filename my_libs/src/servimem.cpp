/*****************************************************************************
  Purpose:     Auxiliary functions working on the heap/stack only.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: SERVIMEM.C,v 1.4 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/


#include <stdlib.h>	/* malloc(), ... */
#include "servimem.h"

#define max_bytes 4     /* a variable length number is composed of max.
                           4 bytes, see the MIDI file specification */


UINT16 swap_int16(UINT16 in) 
{
	return ((in << 8) | (in >> 8)) ;
}


UINT32 swap_int32(UINT32 in) 
{
	INT16 i1 = (INT16)in,
		    i2 = (INT16)(in >> 16) ;
	 
	return (((INT32)swap_int16(i1)) << 16) | swap_int16(i2) ; 
}


unsigned read_var_len(
	const UBYTE *address, UINT32 *read_number, UBYTE *read_bytes)
{
	UINT32 buffer = 0 ;
	unsigned errors = 0 ;

	*read_bytes = 0 ;
	do
	{
		buffer = (buffer << 7) ;
		buffer = buffer | (UINT32)(*address & 0x7f) ;
		address ++ ; 
		*read_bytes +=1 ;
		if (*read_bytes > max_bytes) errors = -1 ;
	} 
	while ( (!errors) && (*(address-1) & 0x80) ) ;
	*read_number = buffer ;
	return(errors) ;
}


UINT32 read_int32(const UBYTE *address)
{
	UINT32 buffer ;

	*((char *)&buffer    ) = *(address    ) ;
	*((char *)&buffer + 1) = *(address + 1) ;
	*((char *)&buffer + 2) = *(address + 2) ;
	*((char *)&buffer + 3) = *(address + 3) ;

#if defined (__PUREC__)
	return(buffer) ;
#else /* Intel */
	return(swap_int32(buffer)) ;
#endif
}


UINT16 read_int16(const UBYTE *address)
{
	UINT16 buffer ;

	*((char *)&buffer    ) = *(address    ) ;
	*((char *)&buffer + 1) = *(address + 1) ;

#if defined (__PUREC__)
	return(buffer) ;
#else /* Intel */
	return(swap_int16(buffer)) ;
#endif
}

char *slist_str(const char *slist, const char *str) 
{
	size_t len ;
	char *s = (char *)slist ;
	
	if (!slist || !str) return 0 ;

	while(1)
	{
		len = strlen(s) ;
		if ( !strcmp(s, str) )
			if ( !s[len] ) 	/* full match till end of string ? */
				return s ;	/* yes ! */
		s += len ;
		if (!s[1]) break ;
		s++ ;
	}
	return 0 ;	/* not found */
}

char *slist_dup(const char *slist) 
{
	size_t len ;
	char *new_list ;
	
	if (!slist) return 0 ;
	
	len = slist_len(slist) + 1 ;
	if ( new_list = (char *)malloc(len) )
		memcpy(new_list, slist, len) ;
	return new_list ;
}

size_t slist_len(const char *slist) 
{
	size_t len, tot_len = 0 ;
	const char *s = slist ;
	
	if (!slist) return 0 ;
	
	while(1)
	{
		len = strlen(s) ;
		tot_len += (len+1) ;
		s += len ;
		if (!s[1]) break ;
		s++ ;
	}
	return tot_len ;
}

