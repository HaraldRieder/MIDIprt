/*****************************************************************************
  Purpose:     MIDI file functions and functions for the MIDI serial interface
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: MIDI.C,v 1.12 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#if defined (__PUREC__)
# include <tos.h>
# define MIDI 3
#endif
#if !defined (__PALM__)
# include <string.h>
# include <ctype.h>
# include <stdio.h>
# include "servimem.h"
#endif
#include "midi.h"


#ifndef min
	#define min(A,B) (((A) < (B)) ? (A) : (B))
#endif
#ifndef max
	#define max(A,B) (((A) > (B)) ? (A) : (B))
#endif

#if defined (__PALM__)

/*static Int32 timeout ;*/

Boolean serial_data_available(void)
{
  if (!port_is_open)
    return false ;
    
/*  if (!timeout)
    // 0.2 seconds timeout allows sufficiently fast reaction on user events
    timeout = SysTicksPerSecond() / 5 ;
  Atari TOS-compatible: return at once (otherwise sustain_late_off called
  too late in MIDI enhancer) */
  recv_error = SerReceiveWait(port_id, 1 /*byte*/, 0/*timeout*/) ;
  return (recv_error == errNone) ;
}

void serial_out(UBYTE byte) 
{
  if (port_is_open) 
    SerSend(port_id, &byte, 1, &send_error) ; 
}

UBYTE serial_in(void) 
{
  UBYTE byte = 0 ;
  if (port_is_open)
    SerReceive(port_id, &byte, 1, 0/*timeout*/, &recv_error) ;
  return byte ;
}

/*void Midiws(UInt32 last_index, const UBYTE *to_send)
{
  UInt32 i ;
  for (i = 0 ; i <= last_index ; i++)
    midi_out(to_send[i]) ;
}*/

#endif

#if defined (__PUREC__) || defined (__PALM__)

void note_on(UBYTE channel, UBYTE note, UBYTE dynamic) 
{
  /* Midiws faster on Palm ? */
  char str[3] ;
  str[0] = NOTE_ON | channel ;
  str[1] = note ;
  str[2] = dynamic ;
  Midiws(2, str) ;
}

void note_off(UBYTE channel, UBYTE note, UBYTE dynamic) 
{
  char str[3] ;
  str[0] = NOTE_OFF | channel ;
  str[1] = note ;
  str[2] = dynamic ;
  Midiws(2, str) ;
}

void pitch_bend(UBYTE channel, UINT16 value)
{
  char str[3] ; 
  str[0] = PITCH_BEND | channel ;
  str[1] = value & 0x007F ;
  str[2] = (value >> 7) & 0x007F ;
  Midiws(2, str) ;
}

void pitch_bend_2(UBYTE channel, UBYTE _first, UBYTE _second)
{
  char str[3] ;
  str[0] = PITCH_BEND | channel ;
  str[1] = _first ;
  str[2] = _second ;
  Midiws(2, str) ;
}

void control_change(UBYTE channel, UBYTE controller, UBYTE value)
{
  char str[3] ;
  str[0] = CONTROL_CHANGE | channel ;
  str[1] = controller ;
  str[2] = value ;
  Midiws(2, str) ;
}

void sustain_on(UBYTE channel)
{
	control_change(channel, SUSTAIN, 0x7F) ;
}

void sustain_off(UBYTE channel)
{
	control_change(channel, SUSTAIN, 0x00) ;
}

void all_notes_off(UBYTE channel)
{
	control_change(channel, ALL_NOTES_OFF, 0x00) ;
}

void program_change(UBYTE channel, UBYTE value)
{
  char str[2] ;
  str[0] = PROGRAM_CHANGE | channel ;
  str[1] = value ;
  Midiws(1, str) ;
}

void channel_press(UBYTE channel, UBYTE value)
{
  char str[2] ;
  str[0] = CHANNEL_PRESS | channel ;
  str[1] = value ;
  Midiws(1, str) ;
}

void poly_key_press(UBYTE channel, UBYTE note, UBYTE value)
{
  char str[3] ;
  str[0] = POLY_KEY_PRESS | channel ;
  str[1] = note ;
  str[2] = value ;
  Midiws(2, str) ;
}


static char tochar(char c)
{
	if ('0' <= c && c <= '9') return (c - '0') ;
	if ('a' <= c && c <= 'z') return (c - 'a' + 0xa) ;
	if ('A' <= c && c <= 'Z') return (c - 'A' + 0xa) ;
	return 0 ;
}

char *ascii_to_midi(char *string, UINT16 *midi_len)
{
	static char err_text[40] ;
	char *src, *dest ;
	unsigned long len ;

	*midi_len = 0 ;	/* init: no valid bytes found */

	/* compress string, ignore all non-hex characters */
	for (src = dest = string ; 
	     *src && *src != '#' && *src != ';' ; 
	     src++)
	{
		if ( isxdigit(*src) )
			*dest++ = *src ;
		else switch (*src)
		{
		  case ' ':
		  case ',':
		  case '\t':
		  case '\r':
		  case '\n':
		    break ;
		  default:
			  /* not hex and none of the allowed serparators */
			  sprintf(err_text, "Invalid char. '%c' at pos. %u.",
				  *src, (unsigned)(src-string)) ;
			  return err_text ;
		}
	}
	*dest = 0 ;
			
	if (! *string)
		return NULL ; /* is empty = already converted */
		
	/* test on even number of nibbles */
	len = strlen(string) ;
	if ( len & 0x1 )
	{
		strcpy(err_text, "Number of nibbles is odd.") ;
		return err_text ;
	}
	*midi_len = len >> 1 ;	/* one byte per 2 hexdigits */
		
	/* convert from ascii to hex */
	for (src = dest = string ; *src ; src += 2, dest++)
		*dest = tochar(src[0]) * 16 + tochar(src[1]) ;
	return NULL ; /* good */
}

static UBYTE pb_sens_message[13] = 
{
	0x00,       /* the status */
	0x64,0x00,	/* RPN parameter number LSB = 0 */
	0x65,0x00,  /* RPN parameter number MSB = 0 */
	            /* parameter number 0 is pitch bend sensitivity */
	0x06,0x00,	/* parameter value MSB */
	0x26,0x00,	/* parameter value LSB */
	0x64,0x7F,	/* reset RPN parameter number to "invalid" */
	0x65,0x7F
} ;
 		
void pitch_bend_sensitivity(UBYTE channel, UINT16 value)
{
	/* uses running status Bn (control change on channel n) */
	/* RPN = real time parameter number                     */
	pb_sens_message[0] = 0xB0 | (channel & 0xF) ; /* the status */
	pb_sens_message[6] = (value>>7) & 0x7F ;      /* parameter value MSB */
	pb_sens_message[8] =  value     & 0x7F ;      /* parameter value LSB */
	Midiws(12, pb_sens_message) ;
}

UBYTE Roland_checksum(UBYTE *begin, UBYTE *end)
{
	UBYTE total = 0 ;
	UBYTE mask  = 0x7F ;
	while ( begin <= end )
	{
		total += *begin ;
		begin++ ;
	}	
	return (0x80 - (total & mask)) & mask ;
}

/******************************/
/**** scale tune functions ****/
/******************************/

typedef unsigned char STUNE[12] ;

#define STUNE_LENGTH 22     /* length of sysex scale tune string */
static UBYTE stune[STUNE_LENGTH] ;
static UBYTE *stune_addr = stune + 5 ; /* where address stands */
static UBYTE *stune_data = stune + 8 ; /* where tuning data starts */

/* non-existing MIDI channel 16 is used for addressing patch data */
#define STUNE_PATCH_CHANNEL 16

void scale_tune_send(UBYTE channel)
{
	if      ( channel < 9 ) stune[6] = 0x11 + channel ;
	else if ( channel > 9 ) stune[6] = 0x10 + channel ;
	else   /* channel 9 */  stune[6] = 0x10 ;
		
	/* GS data set 1 messages have 3 byte address */
	stune[STUNE_LENGTH - 2] = Roland_checksum(stune_addr, stune_data + 11) ;
	Midiws(STUNE_LENGTH - 1, stune) ;
}

void scale_tune_send_all(void)
{
	UBYTE channel ;
	for (channel = 0 ; channel <= STUNE_PATCH_CHANNEL ; channel++)
		scale_tune_send(channel) ;	
}

void scale_tune_init(void) 
{	
	stune[0] = SYS_EX ;
	stune[1] = ROLAND ;
	stune[2] = BROADCAST_DEV ;
	stune[3] = GS_MODEL_ID ;
	stune[4] = DATA_SET_1 ;
	stune[5] = 0x40 ;       /* address MSB is always equal */
	stune[7] = 0x40 ;       /* address LSB is always equal  */
	memset(stune_data, MEAN_STUNE, sizeof(STUNE)) ;
	stune[STUNE_LENGTH - 1] = SYS_EX_END ;
}

void scale_tune_set_cent(INT16 cent[12])
{
	/* up to now: 1 data string for all channels together */
	unsigned i ;
	
	for (i = 0 ; i < 12 ; i++)
		stune_data[i] = min( max(cent[i],-64), 63 ) + MEAN_STUNE ;
}

void scale_tune_set_equal_cent(INT16 cent)
{
	scale_tune_set_equal(min( max(cent,-64), 63 ) + MEAN_STUNE) ;
}

void scale_tune_set(UBYTE stune[12])   { memcpy(stune_data,stune,12); }
void scale_tune_set_equal(UBYTE stune) { memset(stune_data,stune,12); }


#endif /* PURE C or Palm */

#if !defined (__PALM__)

/*****************************/
/**** MIDI file functions ****/
/*****************************/

MIDI_FILE_HEADER read_midi_file_header(const unsigned char *RAM_file)
{
	UINT16 division = 0 ; 
	MIDI_FILE_HEADER header ;

	/* evaluate header chunk */
	if ( memcmp("MThd", RAM_file, 4) )
	{
		/* no correct MIDI file format */
		header.midi_file_type = -1 ; 
		return header ;
	}

	/* continue evaluating header chunk: */
	header.midi_file_type = read_int16(RAM_file + 4 + 4) ;
	header.number_tracks  = read_int16(RAM_file + 4 + 4 + 2) ; 
	division              = read_int16(RAM_file + 4 + 4 + 2 + 2) ;
	if (division & 0x1000)
	{
		/* SMPTE format */
		header.smpte           = (-division) >> 8 ;
		header.ticks_per_frame = division & 0xff ;
		header.ticks_per_beat  = 0 ;
	}
	else 
	{
		/* non-SMPTE format */
		header.smpte           = 0 ;
		header.ticks_per_frame = 0 ;
		header.ticks_per_beat  = division ;
	}
	return header ;
}

#endif