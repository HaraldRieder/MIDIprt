#---------------------------------------------------------------------
#
# $RCSfile: Makefile,v $
# $Revision: 1.1 $     $State: Exp $     $Date: 2008/10/04 15:13:03 $
# $Author: Harald $
#
#---------------------------------------------------------------------
# Description:
#    Cleans up intermediate production results and zips together
#    variants of binary and source distributions.
#---------------------------------------------------------------------
#

default:
	@-echo "Targets are:"
	@-echo "  clean: removes any production intermediate files"
	@-echo "  mfpsrc: zip file for source distribution"
	@-echo "  mfptos: zip file for Atari TOS binary distribution"
	@-echo "  mfpwin: zip file for 32 bit Windows binary distribution"

clean:
	( cd src ; make clean ; cd .. )

mfpsrc: clean
	@-rm -f ../midipsrc.zip
	( cd .. ; zip -r midipsrc.zip MIDIprt/license.txt MIDIprt/src )

mfptos: 
	@-rm -f ../midiptos.zip
	( cd .. ; zip -r midiptos.zip MIDIprt/SCHEMES MIDIprt/HELP MIDIprt/examples MIDIprt/*.AM MIDIprt/*.am MIDIprt/*.APP MIDIprt/*.HTM MIDIprt/*.htm MIDIprt/*.TXT MIDIprt/*.txt )

mfpwin:
	@-rm -f ../midipwin.zip
	( cd .. ; zip -r midipwin.zip MIDIprt/SCHEMES MIDIprt/HELP MIDIprt/examples MIDIprt/*.EXE MIDIprt/*.exe MIDIprt/*.BMP MIDIprt/*.bmp MIDIprt/*.HTM MIDIprt/*.htm MIDIprt/*.TXT MIDIprt/*.txt )
	  
