###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
# Revision 1.11  2009-06-15 11:06:24  mstapelberg
# fix DMA_MEM = 1 instead of 0, split up into readDMA and readDMAFIFO (similar for write), make testParallelABB -f use the FIFO
#
# Revision 1.10  2009-05-29 13:39:06  marcus
# * waitChannel() is now public (needed by the DoubleBuffer in the new bufmgr).
# * removed testMPRACE2loop (it is now just testMPRACE2)
#
# Revision 1.9  2009-04-20 09:53:10  mstapelberg
# Actually talk to the hardware in the IPC test
#
# Revision 1.6  2008-10-10 14:02:41  marcus
# Sync address issues with the latest FPGA designs.
# New testMPRACE2loop proram, long test run.
# sync testMPRACE2 version with the code in the testnode.
#
# Revision 1.5  2008-07-17 12:14:12  marcus
# Added register and changes for test MPRACE2 (debug commit).
#
# Revision 1.4  2008-01-11 10:31:56  marcus
# Added testABBlong.
#
# Revision 1.3  2007-10-31 15:46:11  marcus
# Added IG and KERNEL_PIECES tests.
#
# Revision 1.2  2007-05-29 07:50:43  marcus
# Backup commit.
#
# Revision 1.1  2007/02/12 18:09:13  marcus
# Initial commit.
#
###############################################################

# Makefile for the mprace library test programs

include ../../common.mk

INCDIR += ../../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -lpcidriver -lmprace -lpthread

BINARIES = testABB testABBlong testig testSGDMA testMPRACE2 debugMPRACE2 testParallelABB testFIFO testDGen testParallelFIFO mini-write-pio mini-read-pio mini-write-dma mini-read-dma testDMAInterrupts testOffset v6dmatest testGetDesignID test_reset_timeout testSendDescriptorlist testBuffersizes min_testSendDescriptorList
#testParallelABB testIPCserver testIPCclient
#testABBconfig

###############################################################
# Target definitions

.PHONY: all dirs depend clean

all: dirs depend $(BINARIES)

# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# testDGen has to be linked with the DataGenerator "library"
$(BINDIR)/testDGen: $(OBJDIR)/testDGen.o $(OBJDIR)/DataGenerator.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $(BINDIR)/testDGen $(OBJDIR)/testDGen.o $(OBJDIR)/DataGenerator.o

$(BINDIR)/testParallelFIFO: $(OBJDIR)/testParallelFIFO.o $(OBJDIR)/DataGenerator.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $(BINDIR)/testParallelFIFO $(OBJDIR)/testParallelFIFO.o $(OBJDIR)/DataGenerator.o



# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $@ $<

clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
