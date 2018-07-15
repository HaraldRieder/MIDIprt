#---------------------------------------------------------------------
#
# $RCSfile: Makefile,v $
# $Revision: 1.6 $     $State: Exp $     $Date: 2008/10/04 13:33:27 $
# $Author: Harald $
#
#---------------------------------------------------------------------
# Description:
#    Makefile for GNU Make and Linux.
#    The method for generation of dependencies is described
#    here: http://make.paulandlesley.org/autodep.html 
#---------------------------------------------------------------------
#

LIB=../lib
BIN=../bin
OBJ=../obj
DEP=../dep
SRC=.

TARGET=$(LIB)/libmy.a

CSOURCES=colcube.c diskfile.c graphic.c graphicf.c midi.c notimtab.c servimem.c
CPPSOURCES=wxVDI.cpp 

# wxWidgets gtk2-unicode-release-2.8
CFLAGS= -I../include `wx-config --cppflags`

# tools
C=g++
LINK=ar -rc

# with path
COBJECTS = $(CSOURCES:%.c=$(OBJ)/%.o)
CPPOBJECTS = $(CPPSOURCES:%.cpp=$(OBJ)/%.o)

default:
	@-echo "Targets are:"
	@-echo "  dirs: generates necessary subdirs for object files, ..."
	@-echo "  $(TARGET): produces the UNIX variant of my library"
	@-echo "  all: makes all together"
	@-echo "  clean: removes any intermediate files"
	@-echo "  fullclean: removes also $(TARGET)"

all: dirs $(TARGET)

$(TARGET): $(COBJECTS) $(CPPOBJECTS)
	@rm -f $@
	@-echo "+++ creating static library $@  +++"
	$(LINK) $@ $(COBJECTS) $(CPPOBJECTS)

dirs:
	@-if [ ! -d $(OBJ) ];then \
	  echo "+++ creating directory $(OBJ) +++"; \
	  mkdir $(OBJ); \
	fi
	@-if [ ! -d $(LIB) ];then \
	  echo "+++ creating directory $(LIB) +++ ";\
	  mkdir $(BIN);\
	fi
	@-if [ ! -d $(DEP) ];then \
	  echo "+++ creating directory $(DEP) +++ ";\
	  mkdir $(DEP);\
	fi

clean:
	@-rm -rf $(OBJ) $(DEP) core*
	@# from Windows and Atari production
	@rm -f *.O *.o *.PDB *.LST *.ST? *.BSC *.bsc *.ncb *.opt *.plg *.bak *.suo *.user

fullclean: clean
	@-rm -rf $(TARGET) 

$(OBJ)/%.o: $(SRC)/%.c
	$(C) -c $(CFLAGS) -MMD -o $@ $<
	@cp $(OBJ)/$*.d $(DEP)/$*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(OBJ)/$*.d >> $(DEP)/$*.P; \
	rm -f $(OBJ)/$*.d

$(OBJ)/%.o: $(SRC)/%.cpp 
	$(C) -c $(CFLAGS) -MMD -o $@ $<
	@cp $(OBJ)/$*.d $(DEP)/$*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(OBJ)/$*.d >> $(DEP)/$*.P; \
	rm -f $(OBJ)/$*.d

# include the dependency files
-include $(CSOURCES:%.c=$(DEP)/%.P)
-include $(CPPSOURCES:%.cpp=$(DEP)/%.P)
