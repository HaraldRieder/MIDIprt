/*****************************************************************************
  Purpose:     File access functions (copy on disk, to RAM, ...)
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DISKFILE.C,v 1.5 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifdef __PUREC__

#include <acs.h>        /* because of NULL, Ax_malloc() */
#include <acsplus.h>
#include <ext.h>        /* because of findfirst(), struct ffblk */
#include <string.h>     /* because of strcat(), ... */
#include <stdlib.h> 	/* free(), ... */
#include "..\include\acs_plus.h"
#include "..\include\diskfile.h"
#include "..\include\servimem.h"
#include "diskfile.ah"


void unload_file(void *RAM_file) { free(RAM_file) ; }

void *load_file( char path[], long *size )
{
	long handle, sz ;
	void *RAM_file ;
	size = size ? size : &sz ;
	
	if ( (*size = Af_length(path)) < 0)
		return NULL ;

	/* allocate RAM */
	if ( !(RAM_file = malloc(*size)) )
	{
		my_alert_str(FILE_TOO_LARGE, path) ; 
		return NULL ; 
	}
	
	/* open file, load into RAM, and close file */
	switch ( handle = Fopen(path, FO_READ) )
	{
	case -33: my_alert_str(ACSblk->description->mess[AD_OPEN_STR], path) ; break ;
	case -35: my_alert_str(NO_MORE_HANDLE, NULL) ; break ;
	case -36: my_alert_str(ACCESS_DENIED , path) ; break ; 
	default : 
		/* good case: load file into RAM */
		if ( Fread((int)handle, *size, RAM_file) < 0 )
			my_alert_str(ACSblk->description->mess[AD_READ_STR], path) ;
		if ( Fclose((int)handle) == -37 ) 
			my_alert_str(FILE_NOT_CLOSED, path) ;
		return RAM_file ;
	}
	unload_file(RAM_file) ;
	return NULL ;		/* no success */
}


unsigned int get_filename(char whole_path[], char path[], 
                          char file[], char title[])
{                                     
int button ;
int success ;

	file[0] = 0 ;
	if (_GemParBlk.global[0] >= 0x0140)
		/*** TOS version 1.04 or higher has the fsel_exinput function ***/
		success = fsel_exinput(path, file, &button, title) ;
	else
	{ 
		/*** TOS < 1.04 does not have the fsel_exinput() function, ***/
		/*** simulate the title by an alert box.                   ***/
		if ( title && title[0] )
		{
			/*** title is not empty ***/
			my_alert_str(GET_FILE_NAME, title) ; 
			success = fsel_input(path, file, &button) ;
		}
	}
	if (!button || !file[0]) 
		/*** CANCEL selected or filename empty ***/
		return(no_file_selected) ;
	if (!success)
		/*** error during file select occurred ***/
		return(file_select_error) ;
	/*** take away the file mask (e.g. *.*) ***/
   	strcpy(whole_path, path) ;
	*(strrchr(whole_path, '\\') + 1) = 0 ;    
	strcat(whole_path, file) ;
	return(valid_file_selected) ;
}

static char *fsl_masks = NULL ;
static char *fsl_paths = NULL ;

void fsl_set_masks(const char *slist)
{
	if (fsl_masks) free(fsl_masks) ;
	
	if (slist) fsl_masks = slist_dup(slist) ;
	else       fsl_masks = NULL ;
}

void fsl_add_path(const char *path) 
{
	size_t path_len, old_paths_len ;
	char *new_fsl_paths ;
	char *p ;
	
	if (!path) return ;
	
	path_len = strlen(path) + 1 ;
	
	if (fsl_paths)
	{
		if ( p = slist_str(fsl_paths, path) ) 
		{
			if (p == fsl_paths)
				/* is already contained at 1. position */
				return ;
			/* resort: put path at 1. position */
			memcpy(fsl_paths + path_len, fsl_paths, (p - fsl_paths)) ;
			memcpy(fsl_paths, path, path_len) ;
			return ;
		}
		old_paths_len = slist_len(fsl_paths) + 1 ;
		if ( new_fsl_paths = Ax_malloc(old_paths_len + path_len) )
		{
			strcpy(new_fsl_paths, path) ;
			memcpy(new_fsl_paths + path_len, fsl_paths, old_paths_len) ;
			Ax_free(fsl_paths) ;
			fsl_paths = new_fsl_paths ;
		}
	}
	else if ( fsl_paths = Ax_malloc(path_len + 1) )
	{
		strcpy(fsl_paths, path) ;
		fsl_paths[path_len] = 0 ;
	}
}

char *fsl_do(const char *title) 
{
	static char  whole_path[256] ;
	static INT16 sortmode = SORTBYNAME;
	static char  fname[64] ;
	static char  last_mask[10] ;
	char *s ;
	int	result;
	int button;
	int nfiles;
	char *pattern;
	void *fsel_dialog;
	int whdl;
	EVNT evnt;
	int check, dummy ;
	char path[192] ;
	char *tit = title ? (char *)title : "" ;
	
/*	if (!fsl_masks) fsl_set_masks("\0") ;
	if (!fsl_paths) fsl_add_path("\\") ;
*/	
	/* if mask has changed, clear file name */
	if ( strncmp(last_mask, fsl_masks, 9) )
		*fname = 0 ;
	strncpy(last_mask, fsl_masks, 9) ;
	last_mask[9] = 0 ;
	
	appl_getinfo(7, &check, &dummy, &dummy, &dummy) ;
	if (check & 0x8)
	{
		strcpy(path, fsl_paths) ;
		/* 1. one in list equals path, skip it! */
		s = fsl_paths + strlen(fsl_paths) ;
		if (s[1])	/* another path in list ? */
			s++ ;	/* start with 2. path */
		fsel_dialog = fslx_do( tit,
			path, sizeof(path),	fname, sizeof(fname),
			fsl_masks, 0L /* no filter */, s,
			&sortmode, 
			0, 
			&button, 
			&nfiles, 
			&pattern );
/*		fsel_dialog = fslx_open( tit,
			-1,-1, /* x,y */
			&whdl,
			path, sizeof(path),	fname, sizeof(fname),
			fsl_masks, 0L /* no filter */, fsl_paths,
			sortmode, 0);*/
		if ( fsel_dialog )
		{
/*			Amo_unbusy() ;
Geht nicht im Fenster aus noch ungekl„rter Ursache
Pfade, Masken ?
			do	{
				evnt.mwhich = evnt_multi(
						MU_KEYBD+MU_BUTTON+MU_MESAG,
						2,1,1,
						0,NULL,
						0,NULL,
						evnt.msg,
						0L,
						(EVNTDATA*) &(evnt.mx),
						&evnt.key,
						&evnt.mclicks);
				result = fslx_evnt(
						fsel_dialog,
						&evnt,
						path, fname,
						&button,
						&nfiles,
						&sortmode,
						&pattern);
			} while(result);*/
		
			if ( button )
			{
				/* OK pressed */
				if (s = strrchr(path, '\\'))
					s[1] = 0 ;
				fsl_add_path(path) ;
				strcpy(whole_path, path);
				strcat(whole_path, fname);
				fslx_close(fsel_dialog) ;
				return whole_path ;			
			}
			fslx_close(fsel_dialog) ;
		} 
	}
	else /* old TOS functions */
	{
		strcpy(path, strupr(fsl_paths)) ;
		strcat(path, strupr(fsl_masks)) ;
		if ( get_filename(whole_path, path, fname, tit) == valid_file_selected )
		{
			/* remember selected path and selected mask */
			if (s = strrchr(path, '\\'))
			{
				s++ ; /* position to the mask */
				/* transform new mask to an slist with \0\0 at the 
				   end and store it */
				s[strlen(s) + 1] = 0 ;
				fsl_set_masks(s) ;
				/* split mask away from path and store path */
				*s = 0 ;
				fsl_add_path(path) ;
			}
			return whole_path ;
		}
	}
	return NULL ;
}

void fsl_cleanup(void)
{
	if (fsl_masks)    free(fsl_masks) ; fsl_masks = NULL ;
	if (fsl_paths) Ax_free(fsl_paths) ; fsl_paths = NULL ;
}
       
       
int Fcopy(const char *output_name, const char *input_name)
{
	long long_handle, length ;
	int handle ;
	void *buffer ;

	/* read file content into buffer */
	long_handle = Fopen(input_name, FO_READ) ;
	if (long_handle < 0) return -1 ;
	handle = (int)long_handle ;
	length = filelength(handle) ;
	buffer = malloc(length) ;
	if (!buffer) { Fclose(handle) ; return -1 ; }
	Fread(handle, length, buffer ) ;
	Fclose(handle) ;

	/* write buffer content into file */
	long_handle = Fopen(output_name, FO_WRITE) ;
	if (long_handle < 0)
	{
		/* if not yet existing, create it */
		long_handle = Fcreate(output_name, 0 /* normal file */) ;
		if (long_handle < 0) { free(buffer) ; return -1 ; }
	}
	handle = (int)long_handle ;
	Fwrite(handle, length, buffer) ;
	Fclose(handle) ;
	free(buffer) ;
	return 0 ;
}

#else

#include <stdio.h>

int Fcopy(const char *output_name, const char *input_name)
{
	const unsigned bufsiz = 100 ;
	char buf[bufsiz] ;
	FILE * i = fopen(input_name, "rb") ;
	if (i == NULL)
		return -1 ;
	FILE * o = fopen(output_name, "wb") ;
	if (o == NULL)
	{
		fclose(i) ;
		return -1 ;
	}
	int nread ;
	while ((nread = fread(buf, 1/*byte*/, bufsiz, i)) > 0)
		fwrite(buf, 1, nread, o) ;
	fclose(i) ;
	fclose(o) ;
	return 0 ;
}

#endif