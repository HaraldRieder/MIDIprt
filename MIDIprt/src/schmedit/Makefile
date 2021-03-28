#---------------------------------------------------------------------
#
# $RCSfile: Makefile,v $
# $Revision: 1.5 $     $State: Exp $     $Date: 2008/10/04 15:43:27 $
# $Author: Harald $
#
#---------------------------------------------------------------------
# Description:
#    Makefile for GNU Make and Linux.
#    The method for generation of dependencies is described
#    here: http://make.paulandlesley.org/autodep.html 
#---------------------------------------------------------------------
#

MY_LIBS=../../../my_libs
BIN=../..
OBJ=../obj
DEP=../dep
SRC=.
COMMONSRC=../common

TARGET=$(BIN)/schmedit

CMNSOURCES=scheme.cpp
CPPSOURCES=schmedit.cpp 
LIBS=-lmy -L$(MY_LIBS)/lib `wx-config --libs`

# wxWidgets gtk2-unicode-release-2.8
CFLAGS= -I$(MY_LIBS)/include -I$(COMMONSRC) `wx-config --cppflags`

# tools
C=g++
LD=g++ -g -o
#LD=g++ -o

# with path
CMNOBJECTS = $(CMNSOURCES:%.cpp=$(OBJ)/%.o)
CPPOBJECTS = $(CPPSOURCES:%.cpp=$(OBJ)/%.o)

default: dirs $(TARGET)

$(TARGET): $(CMNOBJECTS) $(CPPOBJECTS)
	@rm -f $@
	@-echo "+++ creating executable $@ +++"
	@$(LD) $(TARGET) $(CMNOBJECTS) $(CPPOBJECTS) $(LIBS) ; 

dirs:
	@-if [ ! -d $(OBJ) ];then \
	  echo "+++ creating directory $(OBJ) +++"; \
	  mkdir $(OBJ); \
	fi
	@-if [ ! -d $(BIN) ];then \
	  echo "+++ creating directory $(BIN) +++ ";\
	  mkdir $(BIN);\
	fi
	@-if [ ! -d $(DEP) ];then \
	  echo "+++ creating directory $(DEP) +++ ";\
	  mkdir $(DEP);\
	fi

clean:
	@-rm -rf $(OBJ) $(DEP) core* Debug Release
	@-rm -f *.O *.o *.PDB *.LST *.ST? *.BSC *.bsc *.ncb *.opt *.plg *.bak *.suo *.user *.aps BuildLog.*

fullclean: clean
	@-rm -rf $(TARGET)

$(OBJ)/%.o: $(COMMONSRC)/%.cpp
	$(C) -c $(CFLAGS) -MMD -o $@ $<
	@cp $(OBJ)/$*.d $(DEP)/$*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(OBJ)/$*.d >> $(DEP)/$*.P; \
	rm -f $(OBJ)/$*.d

$(OBJ)/%.o: $(SRC)/%.cpp 
	$(C) -c $(CFLAGS) -I $(COMMONSRC) -MMD -o $@ $<
	@cp $(OBJ)/$*.d $(DEP)/$*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(OBJ)/$*.d >> $(DEP)/$*.P; \
	rm -f $(OBJ)/$*.d

# include the dependency files
-include $(CMNSOURCES:%.cpp=$(DEP)/%.P)
-include $(CPPSOURCES:%.cpp=$(DEP)/%.P)
