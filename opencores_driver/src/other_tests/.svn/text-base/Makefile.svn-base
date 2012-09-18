###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
# Revision 1.1  2006/11/21 15:54:21  marcus
# Moved board tests to a separate directory, 'other_tests'.
#
# Revision 1.4  2006/11/17 18:56:44  marcus
# Added testCompatInterface.
#
# Revision 1.3  2006/10/30 19:40:32  marcus
# Added testPCIeBoard.
#
# Revision 1.2  2006/10/16 16:57:17  marcus
# Added testCinterface.
#
# Revision 1.1  2006/10/13 17:18:35  marcus
# Implemented and tested most of C++ interface.
#
###############################################################

# Makefile for additional board test programs.

include ../../common.mk

INCDIR += ../../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -lpcidriver

BINARIES = testPCIeBoard testDMA

###############################################################
# Target definitions

.PHONY: all dirs depend clean

all: dirs depend $(BINARIES) $(BINDIR)/testDMA_c

# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $@ $<

$(BINDIR)/testDMA_c: $(OBJDIR)/testDMA_c.o
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $<

clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(BINDIR)/testDMA_c
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
