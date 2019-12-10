/*****************************************************************************
  Purpose:     File access functions (copy on disk, to RAM, ...)
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DISKFILE.H,v 1.4 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef DISKFILE_H 
#define DISKFILE_H 1

/*** general ********************************************************/

#define FILENAME_LENGTH 128  /* max. 127 characters for a file name   */
#define PATHNAME_LENGTH 256  /* max. 255 characters for complete path */

/********************************************************************/


#if defined (__PUREC__)

/* file selector singleton */

void fsl_set_masks(const char *slist) ;
	/* sets search masks for file selector, e.g. "*\0*.app,*.prg\0" */

void fsl_add_path(const char *path) ;
	/* adds path to list of search paths */
	/* if path is already in the list, places path at 1. position
	   of the list */

char *fsl_do(const char *title) ;
	/* file selector, appearance depends on operating system and version */
	/* returns full path to selected file or NULL */

void fsl_cleanup(void) ;
	/* frees and resets any masks or paths allocated by the singleton */
	/* must be called at least once at the end of the application,
	   if any fsl_... function has been called */

/*******************************************************************
*/
	void *load_file( 
		char path[],  /* IN: path of file to be loaded */
		long *length  /* OUT: size of file in RAM (in bytes) */
	) ;
/*
*  Loads a file from disk into RAM. Returns memory pointer when 
*  successful, NULL else. If length is not NULL, the output of 
*  Af_length() is written to it (e.g. "-1" means "is a directory").
*
*******************************************************************/


/*******************************************************************
*/
	void unload_file(
		void *RAM_file	/* begin address of file in RAM */
	) ;
/*
*  Frees the memory occupied by load_file().
* 
*******************************************************************/


/*******************************************************************
*/
	unsigned int get_filename(
		char whole_path[], /* OUT: path + filename */
		char path[],       /* IN:  initial search path for files, incl. search mask 
		                      OUT: path selected by user, incl. search mask */
		char file[],       /* OUT: file selected by user */
		char title[]       /* IN:  TOS >= 1.04 title in the file selector box 
		                           TOS < 1.04 extra title in an alert box, the alert 
		                           is shown only, if title is not empty */
	) ;
/*
*  Opens the file select box and gets the file name from that dialog. 
* 
* RETURN:  
*/
	#define valid_file_selected      0 /* all right */
	#define no_file_selected         1
	#define file_select_error        2 /* severe error ! */
/* 
*******************************************************************/

#endif /* Pure C */

/*******************************************************************
*/
	int Fcopy(
		const char *output_name, /* IN: name of destination file */
		const char *input_name   /* IN: name of source file */
	) ;
/*
*  copies a file using a RAM buffer.
* 
* OUTPUT:  new file on disk
* 
* RETURN:  -1 if no success, 0 else
* 
*******************************************************************/

#endif /* include blocker */

