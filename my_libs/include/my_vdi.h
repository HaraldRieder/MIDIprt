/*      MY_VDI.H

        GEM VDI Definitions
        A subset of the Digital Research Virtual Device Interface
        as used on Atari GEM/TOS for graphics output to a device
        (screen, printer, ...).
        This subset is needed by the MIDI File Printer. The MIDI
        File Printer uses native VDI on Atari platforms and an
        emulation on other platforms. This file defines the interface
        of the emulation layer.
        Native VDI is implemented in C, the emulation layer is 
        implemented in C++. This file is included by files that
        are compiled as C and by files that are compiled as C++ !
*/

#if  !defined( __VDI__ )
#define __VDI__

typedef void * VDI_HANDLE ; // VDI emulation by a class/struct
typedef char * VDI_FONT_ID ;// wxWindows: native font info string

/*
typedef struct
{
    int    contrl[15];
    int    intin[132];
    int    intout[140];
    int    ptsin[145];
    int    ptsout[145];
} VDIPARBLK;

typedef struct
{
    int    *contrl;
    int    *intin;
    int    *ptsin;
    int    *intout;
    int    *ptsout;
} VDIPB;

extern  VDIPARBLK _VDIParBlk;

void vdi( VDIPB *vdipb );
*/

/****** Control definitions *********************************************/

#define RC 2               /* RC coordinate system, by Mad Harry */
#define NDC 0              /* NDC coordinate system, by Mad Harry */
/*
void    v_opnwk( int *work_in,  int *handle, int *work_out);
void    v_clswk( VDI_HANDLE handle );
void    v_opnvwk( int *work_in, int *handle, int *work_out);
void    v_clsvwk( VDI_HANDLE handle );
void    v_clrwk( VDI_HANDLE handle );
void    v_updwk( VDI_HANDLE handle );
int             vst_load_fonts( VDI_HANDLE handle, int select );
void    vst_unload_fonts( VDI_HANDLE handle, int select );
void    vs_clip( VDI_HANDLE handle, int clip_flag, int *pxyarray );
*/

/****** Output definitions **********************************************/

/* emulated */
/* I use void* as handle, native C VDI uses int. The C++ emulation layer
   expects a pointer to a VirtualDevice object as handle. */
void    v_pline( VDI_HANDLE handle, int count, int *pxyarray ); 
void    v_gtext( VDI_HANDLE handle, int x, int y, char *string );
void    v_bar( VDI_HANDLE handle, int *pxyarray );
void    v_circle( VDI_HANDLE handle, int x, int y, int radius );
void    v_ellpie( VDI_HANDLE handle, int x, int y, int xradius,
                  int yradius, int begang, int endang );
void    v_ellipse( VDI_HANDLE handle, int x, int y, int xradius,
                   int yradius  );
void    v_rfbox ( VDI_HANDLE handle, int *pxyarray );
void    v_fillarea( VDI_HANDLE handle, int count, int *pxyarray );
void    v_pmarker( VDI_HANDLE handle, int count, int *pxyarray );
void    v_cellarray( VDI_HANDLE handle, int *pxyarray, int row_length,
                     int el_used, int num_rows, int wrt_mode,
                     int *colarray );
void    v_contourfill( VDI_HANDLE handle, int x, int y, int index );
void    vr_recfl( VDI_HANDLE handle, int *pxyarray );
void    v_arc( VDI_HANDLE handle, int x, int y, int radius,
               int begang, int endang );
void    v_pieslice( VDI_HANDLE handle, int x, int y, int radius,
                    int begang, int endang );
void    v_ellarc( VDI_HANDLE handle, int x, int y, int xradius,
                  int yradius, int begang, int endang );
void    v_rbox  ( VDI_HANDLE handle, int *pxyarray );
void    v_justified( VDI_HANDLE handle,int x, int y, char *string,
                     int length, int word_space,
                     int char_space );

/****** Attribute definitions *****************************************/

/* the first 16 palette colors */

#define WHITE            0
#define BLACK            1
#define RED              2
#define GREEN            3
#define BLUE             4
#define CYAN             5
#define YELLOW           6
#define MAGENTA          7
#define LWHITE           8
#define LBLACK           9
#define LRED            10
#define LGREEN          11
#define LBLUE           12
#define LCYAN           13
#define LYELLOW         14
#define LMAGENTA        15

/* bit masks for vst_effects() */

#define NORMAL      0x00
#define BOLD        0x01
#define LIGHT       0x02
#define ITALIC      0x04
#define UNDERLINED  0x08
#define HOLLOW      0x10
#define SHADOWED    0x20

/* patterns for vsf_style() */

#define IP_HOLLOW       0
#define IP_1PATT        1
#define IP_2PATT        2
#define IP_3PATT        3
#define IP_4PATT        4
#define IP_5PATT        5
#define IP_6PATT        6
#define IP_7PATT        7
#define IP_SOLID        8


/* gsx modes for vswr_mode() */

#define MD_REPLACE      1
#define MD_TRANS        2
#define MD_XOR          3
#define MD_ERASE        4


/* gsx styles for vsf_interior() */

#define FIS_HOLLOW      0
#define FIS_SOLID       1
#define FIS_PATTERN     2
#define FIS_HATCH       3
#define FIS_USER        4


/* bit blt rules for vro_cpyfm() and vrt_cpyfm() */

#define ALL_WHITE        0
#define S_AND_D          1
#define S_AND_NOTD       2
#define S_ONLY           3
#define NOTS_AND_D       4
#define D_ONLY           5
#define S_XOR_D          6
#define S_OR_D           7
#define NOT_SORD         8
#define NOT_SXORD        9
#define D_INVERT        10
#define NOT_D           11
#define S_OR_NOTD       12
#define NOTS_OR_D       13
#define NOT_SANDD       14
#define ALL_BLACK       15


/* linetypes for vsl_type() */

#define SOLID           1
#define LONGDASH        2
#define DOT             3
#define DASHDOT         4
#define DASH            5
#define DASH2DOT        6
#define USERLINE        7

/* line ends for vsl_ends() */

#define SQUARE          0
#define ARROWED         1
#define ROUND           2

/* text effects (for vst_effects) */
#define TF_NORMAL			0x00
#define TF_THICKENED		0x01
#define TF_LIGHTENED		0x02
#define TF_SLANTED			0x04
#define TF_UNDERLINED		0x08
#define TF_OUTLINED			0x10
#define TF_SHADOWED			0x20

/* text alignment (for vst_alignment) */
/* conflict with Windows definitions! */
#ifndef _WINDOWS
#define TA_LEFT			0
#define TA_CENTER		1
#define TA_RIGHT		2

#define TA_BASELINE		0
#define TA_HALF			1
#define TA_ASCENT		2
#define TA_BOTTOM		3
#define TA_DESCENT		4
#define TA_TOP			5
#endif


/******************************************************************************/


int     vsl_type( VDI_HANDLE handle, int style );
int     vsl_width( VDI_HANDLE handle, int width );
int     vsl_color( VDI_HANDLE handle, int color_index );
void    vst_height( VDI_HANDLE handle, int height, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height );
int     vst_point( VDI_HANDLE handle, int point, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height );
const VDI_FONT_ID 
		vst_font( VDI_HANDLE handle, const VDI_FONT_ID font );
int     vst_effects( VDI_HANDLE handle, int effect );
void    vst_alignment( VDI_HANDLE handle, int hor_in, int vert_in,
                       int *hor_out, int *vert_out );
int     vsf_interior( VDI_HANDLE handle, int style );
int     vsf_style( VDI_HANDLE handle, int style_index );
int     vsf_color( VDI_HANDLE handle, int color_index );
int     vsf_perimeter( VDI_HANDLE handle, int per_vis );
int     vswr_mode( VDI_HANDLE handle, int mode );
void    vs_color( VDI_HANDLE handle, int index, int *rgb_in );
void    vsl_udsty( VDI_HANDLE handle, int pattern );
void    vsl_ends( VDI_HANDLE handle, int beg_style, int end_style );
int     vsm_type( VDI_HANDLE handle, int symbol );
int     vsm_height( VDI_HANDLE handle, int height );
int     vsm_color( VDI_HANDLE handle, int color_index );
int     vst_rotation( VDI_HANDLE handle, int angle );
int     vst_color( VDI_HANDLE handle, int color_index );
void    vsf_udpat( VDI_HANDLE handle, int *pfill_pat, int planes );


/****** Raster definitions *********************************************/
/*
typedef struct
{
        void            *fd_addr;
        int             fd_w;
        int             fd_h;
        int             fd_wdwidth;
        int             fd_stand;
        int             fd_nplanes;
        int             fd_r1;
        int             fd_r2;
        int             fd_r3;
} MFDB;

void    vro_cpyfm( VDI_HANDLE handle, int vr_mode, int *pxyarray,
                   MFDB *psrcMFDB, MFDB *pdesMFDB );
void    vrt_cpyfm( VDI_HANDLE handle, int vr_mode, int *pxyarray,
                   MFDB *psrcMFDB, MFDB *pdesMFDB,
                   int *color_index );
void    vr_trnfm( VDI_HANDLE handle, MFDB *psrcMFDB, MFDB *pdesMFDB );
void    v_get_pixel( VDI_HANDLE handle, int x, int y, int *pel,
                     int *index );
*/

/****** Input definitions **********************************************/
/*
void    vsin_mode( VDI_HANDLE handle, int dev_type, int mode );
void    vrq_locator( VDI_HANDLE handle, int x, int y, int *xout,
                     int *yout, int *term );
int     vsm_locator( VDI_HANDLE handle, int x, int y, int *xout,
                     int *yout, int *term );
void    vrq_valuator( VDI_HANDLE handle, int valuator_in,
                      int *valuator_out, int *terminator );
void    vsm_valuator( VDI_HANDLE handle, int val_in, int *val_out,
                      int *term, int *status );
void    vrq_choice( VDI_HANDLE handle, int ch_in, int *ch_out );
int     vsm_choice( VDI_HANDLE handle, int *choice );
void    vrq_string( VDI_HANDLE handle, int max_length, int echo_mode,
                    int *echo_xy, char *string );
int     vsm_string( VDI_HANDLE handle, int max_length, int echo_mode,
                    int *echo_xy, char *string );
void    vsc_form( VDI_HANDLE handle, int *pcur_form );
void    vex_timv( VDI_HANDLE handle, int (*tim_addr)(), int (**otim_addr)(),
                  int *tim_conv );
void    v_show_c( VDI_HANDLE handle, int reset );
void    v_hide_c( VDI_HANDLE handle );
void    vq_mouse( VDI_HANDLE handle, int *pstatus, int *x, int *y );
void    vex_butv( VDI_HANDLE handle, int (*pusrcode)(), int (**psavcode)() );
void    vex_motv( VDI_HANDLE handle, int (*pusrcode)(), int (**psavcode)() );
void    vex_curv( VDI_HANDLE handle, int (*pusrcode)(), int (**psavcode)() );
void    vq_key_s( VDI_HANDLE handle, int *pstatus );
*/

/****** Inquire definitions *******************************************/
/*
void    vq_extnd( VDI_HANDLE handle, int owflag, int *work_out );
int     vq_color( VDI_HANDLE handle, int color_index,
                  int set_flag, int *rgb );
void    vql_attributes( VDI_HANDLE handle, int *attrib );
void    vqm_attributes( VDI_HANDLE handle, int *attrib );
void    vqf_attributes( VDI_HANDLE handle, int *attrib );
void    vqt_attributes( VDI_HANDLE handle, int *attrib );
void    vqt_extent( VDI_HANDLE handle, char *string, int *extent );
int     vqt_width( VDI_HANDLE handle, int character,
                   int *cell_width, int *left_delta,
                   int *right_delta );
int     vqt_name( VDI_HANDLE handle, int element_num, char *name );
void    vq_cellarray( VDI_HANDLE handle, int *pxyarray,
                      int row_length, int num_rows,
                      int *el_used, int *rows_used,
                      int *status, int *colarray );
void    vqin_mode( VDI_HANDLE handle, int dew_type, int *input_mode );
void    vqt_fontinfo( VDI_HANDLE handle, int *minADE, int *maxADE,
                      int *distances, int *maxwidth,
                      int *effects );
*/

/****** Escape definitions *********************************************/
/*
void    vq_chcells( VDI_HANDLE handle, int *rows, int *columns );
void    v_exit_cur( VDI_HANDLE handle );
void    v_enter_cur( VDI_HANDLE handle );
void    v_curup( VDI_HANDLE handle );
void    v_curdown( VDI_HANDLE handle );
void    v_curright( VDI_HANDLE handle );
void    v_curleft( VDI_HANDLE handle );
void    v_curhome( VDI_HANDLE handle );
void    v_eeos( VDI_HANDLE handle );
void    v_eeol( VDI_HANDLE handle );
void    vs_curaddress( VDI_HANDLE handle, int row, int column );
void    v_curaddress( VDI_HANDLE handle, int row, int column );
void    v_curtext( VDI_HANDLE handle, char *string );
void    v_rvon( VDI_HANDLE handle );
void    v_rvoff( VDI_HANDLE handle );
void    vq_curaddress( VDI_HANDLE handle, int *row, int *column );
int     vq_tabstatus( VDI_HANDLE handle );
void    v_hardcopy( VDI_HANDLE handle );
void    v_dspcur( VDI_HANDLE handle, int x, int y );
void    v_rmcur( VDI_HANDLE handle );
void    v_form_adv( VDI_HANDLE handle );
void    v_output_window( VDI_HANDLE handle, int *xyarray );
void    v_clear_disp_list( VDI_HANDLE handle );
void    v_bit_image( VDI_HANDLE handle, const char *filename,
                     int aspect, int x_scale, int y_scale,
                     int h_align, int v_align, int *xyarray );
void    vq_scan( VDI_HANDLE handle, int *g_slice, int *g_page,
                 int *a_slice, int *a_page, int *div_fac);
void    v_alpha_text( VDI_HANDLE handle, char *string );
void    vs_palette( VDI_HANDLE handle, int palette );
void	v_sound( VDI_HANDLE handle, int frequency, int duration );
int		vs_mute( VDI_HANDLE handle, int action );
void    vqp_films( VDI_HANDLE handle, char *film_names );
void    vqp_state( VDI_HANDLE handle, int *port, char *film_name,
                   int *lightness, int *interlace,
                   int *planes, int *indexes );
void    vsp_state( VDI_HANDLE handle, int port, int film_num,
                   int lightness, int interlace, int planes,
                   int *indexes );
void    vsp_save( VDI_HANDLE handle );
void    vsp_message( VDI_HANDLE handle );
int     vqp_error( VDI_HANDLE handle );
void    v_meta_extents( VDI_HANDLE handle, int min_x, int min_y,
                        int max_x, int max_y );
void    v_write_meta( VDI_HANDLE handle,
                      int num_intin, int *intin,
                      int num_ptsin, int *ptsin );
void    vm_coords( VDI_HANDLE handle, int llx, int lly, int urx, int ury );
void    vm_filename( VDI_HANDLE handle, const char *filename );
void    vm_pagesize( VDI_HANDLE handle, int pgwidth, int pdheight );
void    v_offset( VDI_HANDLE handle, int offset );
void    v_fontinit( VDI_HANDLE handle, int fh_high, int fh_low );
void    v_escape2000( VDI_HANDLE handle, int times );

void    vt_resolution( VDI_HANDLE handle, int xres, int yres,
                       int *xset, int *yset );
void    vt_axis( VDI_HANDLE handle, int xres, int yres,
                 int *xset, int *yset );
void    vt_origin( VDI_HANDLE handle, int xorigin, int yorigin );
void    vq_tdimensions( VDI_HANDLE handle, int *xdimension, int *ydimension );
void    vt_alignment( VDI_HANDLE handle, int dx, int dy );
void    vsp_film( VDI_HANDLE handle, int index, int lightness );
void    vsc_expose( VDI_HANDLE handle, int state );
*/

#define GDOS_NONE      -2L            /* no GDOS installed           */
#define GDOS_FSM       0x5F46534DL    /* '_FSM' - FSMGDOS installed  */
#define GDOS_FNT       0x5F464E54L    /* '_FNT' - FONTGDOS installed */
/*
int     vq_gdos( void );
long    vq_vgdos( void );

int     v_bez_on( VDI_HANDLE handle );
void    v_bez_off( VDI_HANDLE handle );
void    v_set_app_buff( VDI_HANDLE handle, void *address, int nparagraphs );
void    v_bez( VDI_HANDLE handle, int count, int *xyarr,
                char *bezarr, int *extent, int *totpts, int *totmoves );
void    v_bez_fill( VDI_HANDLE handle, int count, int *xyarr,
                     char *bezarr, int *extent, int *totpts,
                     int *totmoves );
int     v_bez_qual( VDI_HANDLE handle, int prcnt, int *actual );
*/

/****** SpeedoGDOS definitions ********************************************/
/*
typedef long  fix31;

void    vqt_f_extent( VDI_HANDLE handle, char *string, int *extent );
void    v_ftext( VDI_HANDLE handle, int x, int y, char *string );
void    v_ftext_offset( VDI_HANDLE handle, int x, int y, char *string, int *offset );
void    v_killoutline( VDI_HANDLE handle, void *component );
void    v_getoutline( VDI_HANDLE handle, int ch, int *xyarray,
                      char *bezarray, int maxverts, int *numverts );
void    vst_scratch( VDI_HANDLE handle, int mode );
void    vst_error( VDI_HANDLE handle, int mode, int *errorvar );
void    vqt_advance( VDI_HANDLE handle, int ch, int *advx, int *advy,
                     int *remx, int *remy );
void    vqt_advance32( VDI_HANDLE handle, int ch, fix31 *advx, fix31 *advy );
int     vst_arbpt( VDI_HANDLE handle, int point, int *chwd, int *chht,
                   int *cellwd, int *cellht );
fix31   vst_arbpt32( VDI_HANDLE handle, fix31 point, int *chwd, int *chht,
                     int *cellwd, int *cellht );
void    vst_charmap( VDI_HANDLE handle, int mode );
void    v_getbitmap_info( VDI_HANDLE handle, int ch, fix31 *advx, fix31 *advy,
                          fix31 *xoff, fix31 *yoff, fix31 *width,
                          fix31 *height );
void    vqt_pairkern( VDI_HANDLE handle, int ch1, int ch2, fix31 *x, fix31 *y );
void    vqt_trackkern( VDI_HANDLE handle, fix31 *x, fix31 *y );
void    vqt_fontheader( VDI_HANDLE handle, char *buffer, char *pathname );
void    vst_kern( VDI_HANDLE handle, int tmode, int pmode, int *tracks,
                  int *pairs );
fix31   vst_setsize32( VDI_HANDLE handle, fix31 point, int *chwd, int *chht,
                       int *cellwd, int *cellht );
void    vqt_devinfo( VDI_HANDLE handle, int devnum, int *devexits,
                     char *devstr );
int     v_flushcache( VDI_HANDLE handle );
void    vqt_cachesize( VDI_HANDLE handle, int which_cache, long *size );
void    vqt_get_table( VDI_HANDLE handle, int **map );
int     v_loadcache( VDI_HANDLE handle, char *filename, int mode );
int     v_savecache( VDI_HANDLE handle, char *filename );
int     vst_setsize( VDI_HANDLE handle, int point, int *chwd, int *chht,
                     int *cellwd, int *cellht );
int     vst_skew( VDI_HANDLE handle, int skew );
*/

#endif

/***********************************************************************/
