#---------------------------------------------------------------------
#
# $RCSfile: Makefile,v $
# $Revision: 1.1 $     $State: Exp $     $Date: 2008/10/04 14:12:43 $
# $Author: Harald $
#
#---------------------------------------------------------------------
# Description:
#    Cleans up intermediate production results and zips together
#    Atari TOS and Windows libs and all source files.
#    To produce the library variants use the files in src
#    (and on Atari TOS in acsmodul)!
#---------------------------------------------------------------------
#

TARGET=my_libs.zip

default:
	@-echo "Targets are:"
	@-echo "  clean: removes any production intermediate files"
	@-echo "  ../$(TARGET): produces the zip file for distribution"

clean:
	@-( cd src      ; make fullclean ; cd .. )
	@-( cd lib ; make clean ; cd .. )

../$(TARGET): clean
	@-rm -f $@
	( cd .. ; zip -r $(TARGET) my_libs )
	
