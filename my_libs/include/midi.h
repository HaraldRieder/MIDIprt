/*****************************************************************************
  Purpose:     MIDI file functions and functions for the MIDI serial interface
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: MIDI.H,v 1.7 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#if defined (__PALM__)
#  include <PalmOS.h>
// new serial manager exists from OS version 3.3 on 
#  include <SerialMgrOld.h>
#endif

#ifndef INCL_MIDI
#define INCL_MIDI

/* on Cygnus portable.h is a soft link in the source dir. */
/* use "" instead of <> for including */
#include "portable.h"

#define MSN            0xF0 /* most significant nibble */
#define LSN            0x0F /* least significant nibble */

#define STATUS         0x80 /* most significant bit set in status byte */
#define NOTE_ON        0x90 /* note on status nibble */
#define NOTE_OFF       0x80 /* note off status nibble */
#define MEAN_DYNAMIC   0x40 /* mean note on/off dynamic */
#define POLY_KEY_PRESS 0xA0 /* polyphonic key pressure status nibble */
#define CHANNEL_PRESS  0xD0 /* channel pressure status nibble */
#define PROGRAM_CHANGE 0xC0 /* program change status nibble */

#define CONTROL_CHANGE 0xB0 /* controller change status nibble */
#define BANK_SELECT_MSB   0 /* bank select most significant byte */
#define BANK_SELECT_LSB  32 /* bank select least significant byte */
#define SUSTAIN          64 /* controller number for sustain pedal */
#define ALL_NOTES_OFF   123 /* controller number for "all notes off" */

#define PITCH_BEND     0xE0 /* pitch bend status nibble */
#define MIN_PITCH    0x0000 /* pitch bend min. position */
#define MAX_PITCH    0x3FFF /* pitch bend max. position */
#define MEAN_PITCH   0x2000 /* pitch bend in center position */

#define MIN_STUNE      0x00 /* GS scale tune min. -64 cent */
#define MAX_STUNE      0x7F /* GS scale tune max. +63 cent */
#define MEAN_STUNE     0x40 /* GS scale tune chromatic */

#define SYS_EX         0xF0 /* system exclusive transmit F0 */
#define SYS_EX_NT      0xF7 /* system exclusive not transmit F0 */
#define SYS_EX_END     0xF7 /* end of system exclusive data */

#define ROLAND         0x41
#define BROADCAST_DEV  0x7F /* broadcast device */
#define GS_MODEL_ID    0x42
#define DATA_SET_1     0x12

#define META_EVENT     0xFF /* meta UBYTE */
#define SEQUENCE_NUM   0x00 /* sequence/file number */
#define GENERAL_TEXT   0x01 /* text event */
#define COPYR_NOTICE   0x02 /* copyright notice */
#define SQ_TRCK_NAME   0x03 /* sequence or track name */
#define INSTRUM_NAME   0x04 /* instrument name */
#define LYRIC          0x05 /* lyric event */
#define TEXT_MARKER    0x06 /* marker event */
#define CUE_POINT      0x07 /* event happening on stage, ... */
#define PROGRAM_NAME   0x08 /* together with progr. change ? */
#define DEVICE_NAME    0x09 /* e.g. synthesizer model to use for track */
#define END_OF_TRACK   0x2F 
#define SET_TEMPO      0x51 /* in microseconds */
#define SMPTE_OFFSET   0x54
#define TIME_SIGNATURE 0x58
#define KEY_SIGNATURE  0x59

#if defined (__PUREC__)

# define midi_data_available Bconstat(3)
	/* returns != 0 if a data byte can be fetched 
	   from the MIDI input buffer */
# define midi_out(data) Bconout(3,(UBYTE)data) 
	/* sends 1 data byte to MIDI output */
# define midi_in        (UBYTE)Bconin(3) 
	/* delivers 1 unsigned byte from MIDI input */

#elif defined (__PALM__)

extern UInt16 port_id ;
extern Boolean port_is_open ;
  /* opening/closing the serial port must be 
     performed by the Palm application */

extern Err recv_error ;
extern Err send_error ;
  /* receive errors are reported into these variables */

# define midi_data_available serial_data_available()
# define midi_out(data)      serial_out(data)
# define midi_in             serial_in()
# define Midiws(last_i, str) if (port_is_open) SerSend(port_id, str, (last_i)+1, &send_error)

Boolean serial_data_available(void) ;
  /* tests port_id on available data */

void serial_out(UBYTE) ;
  /* sends 1 byte to serial port */

UBYTE serial_in() ;
  /* receives 1 byte from serial port */

#endif

void note_on       (UBYTE channel, UBYTE note, UBYTE dynamic) ;
void note_off      (UBYTE channel, UBYTE note, UBYTE dynamic) ;
void pitch_bend    (UBYTE channel, UINT16 value) ;
void pitch_bend_2  (UBYTE channel, UBYTE _first, UBYTE _second) ; 
void control_change(UBYTE channel, UBYTE controller, UBYTE value) ;
void sustain_on    (UBYTE channel) ;
void sustain_off   (UBYTE channel) ;
void all_notes_off (UBYTE channel) ;
void program_change(UBYTE channel, UBYTE value) ;
void poly_key_press(UBYTE channel, UBYTE note, UBYTE value) ;
void channel_press (UBYTE channel, UBYTE value) ;

char *ascii_to_midi(char *string, UINT16 *midi_len) ;
	/* assumes string to be an ASCII string of an even 
	   number of hex digits and converts them to their
	   byte values. Number of digits must be even.
	   Blanks, tabs and commas are allowed separators. 
	   Characters after and including '#' or ';' are
	   treated as comment.
	   Returns NULL if good, otherwise English error text.
	   In good case, midi_len is set to the number of 
	   bytes in the transformed string. */

/***********************/
/**** GM extensions ****/
/***********************/

void pitch_bend_sensitivity(UBYTE channel, UINT16 value) ;
	/* sets pitch bend sensitivity to value (14 bit)       */
	/* upper 7 bits are sensitivity in 2^(1/12) = 100 cent */
	/* GS synths ignore lower 7 bits                       */ 
	
/*************************/
/**** Roland specific ****/
/*************************/

UBYTE Roland_checksum(UBYTE *begin, UBYTE *end) ;
	/* Calculates a checksum for Roland System Exclusive messages */
	/* between and including begin and end. Returns the checksum. */


/******************************/
/**** scale tune functions ****/
/******************************/

/* Roland GS compatible scale tune functions. */
/* Scale tune values are allowed between and  */
/* including 0x00 and 0x7F. These correspond  */
/* to -64 to +63 Cent, e.g. 0x40 = 0 Cent.    */

void scale_tune_init(void) ;
	/* Must be called before first use of any other scale tune function !   */
	/* Inits internal data structures and the scale tune to equal temp.     */

void scale_tune_set_cent(INT16 cent[12]) ;
	/* For all channels together sets current scale tune. */
	/* Values < -64 or > +63 are set to -64 and +63 resp. */

void scale_tune_set_equal_cent(INT16 cent) ;
	/* For all channels together sets current scale tune  */
	/* to the same value.                                 */
	/* Values < -64 or > +63 are set to -64 and +63 resp. */

void scale_tune_set(UBYTE stune[12]) ;
	/* For all channels together sets current scale tune. */
	/* This functions has no check ! I.e. values >= 0x80  */
	/* are passed through and may lead to a checksum err. */

void scale_tune_set_equal(UBYTE stune) ;
	/* For all channels together sets current scale tune  */
	/* to the same value.                                 */
	/* This functions has no check ! I.e. values >= 0x80  */
	/* are passed through and may lead to a checksum err. */

void scale_tune_send(UBYTE channel) ;
	/* Sends the current scale tune values on specified MIDI channel. */

void scale_tune_send_all(void) ;
	/* Sends the current scale tune values on all 16 MIDI channels. */

/*****************************/
/**** MIDI file functions ****/
/*****************************/

typedef struct
{
	INT16  midi_file_type ;
	INT16  number_tracks;
	INT16  smpte;			/* >= 0 if SMPTE format */
	INT16  ticks_per_beat;
	INT16  ticks_per_frame;
}
MIDI_FILE_HEADER ;

MIDI_FILE_HEADER read_midi_file_header(const UBYTE *RAM_file);
	/* Reads the header chunk data from a MIDI file in RAM (RAM_file)
	   and returns them in the header struct. 
	   Returns < 0 in midi_file_type if format not OK */

#endif /* include blocker */
