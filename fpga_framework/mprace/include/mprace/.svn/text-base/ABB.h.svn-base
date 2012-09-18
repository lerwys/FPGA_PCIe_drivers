#ifndef ABB_H_
#define ABB_H_

/*******************************************************************
 * Change History:
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2009-06-15 11:06:24  mstapelberg
 * fix DMA_MEM = 1 instead of 0, split up into readDMA and readDMAFIFO (similar for write), make testParallelABB -f use the FIFO
 *
 * Revision 1.8  2009-04-17 15:37:40  marcus
 * * Added multiple PCI_BAR DMA support, needed for the new ABB FIFO tests.
 * * Added MAIN_LOOPBACK flags, in order to enable/disable the neccesary changes for loopback tests.
 *
 * Revision 1.7  2008-07-17 12:14:12  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.6  2008-01-11 10:30:22  marcus
 * Modified Interrupt mechanism. Added an IntSource parameter. This adds experimental support for concurrent interrupts, and handles better some race conditions in the driver.
 *
 * Revision 1.5  2007-10-31 15:48:33  marcus
 * Added IG and KERNEL_PIECES tests.
 * Added Register access functions.
 *
 * Revision 1.4  2007-07-06 19:20:37  marcus
 * New Register map for the ABB.
 * User Memory Support with Scatter/Gather Lists.
 *
 * Revision 1.3  2007-05-29 07:50:48  marcus
 * Backup commit.
 *
 * Revision 1.2  2007/03/02 14:58:25  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:18  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "DMAEngineWG.h"
#include "Board.h"
#include "Exception.h"
#include "InterruptGenerator.h"

namespace mprace {

class DMABuffer;
class Register;

/**
 * Implements the library interface for the ABB Test Board.
 *
 * @author  Guillermo Marcus
 * @version $Revision: 1.10 $
 * @date    $Date: 2009-06-15 12:29:02 $
 */
class ABB : public Board {
public:
	/**
	 * Probes a Driver if it handles an ABB Board.
	 * @param device The device to probe.
	 * @return true if is an ABB board, false otherwise.
	 */
	static bool probe(Driver& dev);

	/**
	 * Creates an ABB board object.
	 * @param number The number of the board to initialize
	 * @todo How are boards enumerated?
	 */
	ABB(const unsigned int number);

	/**
	 * Releases a board.
	 */
	virtual ~ABB();


	/**
	 * Write a value to a register in the board.
	 *
	 * @param address Offset (address) in the register space.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void setReg(const unsigned int address, const unsigned int value);

	/**
	 * Read a value from a register in the board.
	 *
	 * @param address Offset (address) in the register space.
	 * @return The value read from the register.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int getReg(const unsigned int address);

	/**
	 * Write a value to the board address space.
	 *
	 * @param address Address in the board.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void write(const unsigned int address, const unsigned int value);

	/**
	 * Write a value to the board address space.
	 * 
	 * @param address Address in the board.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void writeFIFO(const unsigned int address, const unsigned int value);


	/**
	 * Read a value from the board address space.
	 *
	 * @param address Address in the board.
	 * @return The value read from the board.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int read(const unsigned int address);

	/**
	 * Read a value from the board address space.
	 * 
	 * @param address Address in the board.
	 * @return The value read from the board.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int readFIFO(const unsigned int address);


	/**
	 * Write multiple values to the board address space.
	 *
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @exception mprace::Exception On error.
	 */
	inline void writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc = true );

	/**
	 * Read multiple values from the board address space.
	 *
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @exception mprace::Exception On error.
	 */
	inline void readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc = true );

	/**
	 * Write multiple values from a DMA Buffer to the board address space.
	 *
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @exception mprace::Exception On error.
	 */
	virtual void writeDMA(const unsigned int address, const DMABuffer& buf,
			const unsigned int count, const unsigned int offset =
			0, const bool inc = true, const bool lock = true, const
			float timeout = 0.0);

	/**
	 * Write multiple values from a DMA Buffer to the board's FIFO address space.
	 *
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @exception mprace::Exception On error.
	 */
	virtual void writeDMAFIFO(const unsigned int address, const DMABuffer&
			buf, const unsigned int count, const unsigned int
			offset = 0, const bool inc = true, const bool lock =
			true, const float timeout = 0.0 );


	/**
	 * Read multiple values from the board address space to a DMA Buffer.
	 *
	 * WARNING: If you pass lock = false, you need to manually issue
	 * buf.sync(DMABuffer::FROMDEVICE); after the DMA transaction!
	 *
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @exception mprace::Exception On error.
	 */
	virtual void readDMA(const unsigned int address, DMABuffer& buf,
			const unsigned int count, const unsigned int offset = 0,
			const bool inc = true, const bool lock = true,
			const float timeout = 0.0);

	/**
	 * Read multiple values from the board's FIFO address space to a DMA Buffer.
	 *
	 * WARNING: If you pass lock = false, you need to manually issue
	 * buf.sync(DMABuffer::FROMDEVICE); after the DMA transaction!
	 *
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @exception mprace::Exception On error.
	 */
	virtual void readDMAFIFO(const unsigned int address, DMABuffer& buf,
			const unsigned int count, const unsigned int offset = 0,
			const bool inc = true, const bool lock = true,
			const float timeout = 0.0);


	/**
	 * Get the DMA Engine for this board.
	 */
	inline DMAEngine& getDMAEngine() { return *dma; }

	/**
	 * Get the Interrupt Generator module.
	 *
	 * @return mprace::InterruptGenerator, or NULL if not present
	 */
	inline InterruptGenerator& getInterruptGenerator() { return *ig; }

#if 0
	/**
	 * Configure the Main FPGA.
	 *
	 * @param filename File name of the configuration file. Can be in BIT, BIN or RBT format.
	 * @return Board:ConfigStatus Configuration Status
	 * @exception mprace::Exception On error.
	 */
	virtual inline Board::ConfigStatus config( const std::string& filename )
	{ throw mprace::Exception( mprace::Exception::UNKNOWN); }
//	{ return v4config->config(filename); }

	/**
	 * Configure the Main FPGA.
	 *
	 * @param filename File name of the configuration file. Can be in BIT, BIN or RBT format.
	 * @return Board:ConfigStatus Configuration Status
	 * @exception mprace::Exception On error.
	 */
	virtual inline Board::ConfigStatus config( const char *filename )
	{ throw mprace::Exception( mprace::Exception::UNKNOWN); }
//	{ return v4config->config(filename); }

	/**
	 * Configure the Main FPGA.
	 *
	 * @param byteCount Size of the bitstream, in bytes.
	 * @param bitstream Bytes of the bitstream.
	 * @return Board:ConfigStatus Configuration Status
	 * @exception mprace::Exception On error.
	 */
	virtual inline Board::ConfigStatus config( unsigned int byteCount, const char *bitstream )
	{ throw mprace::Exception( mprace::Exception::UNKNOWN); }
//	{ return v4config->config(byteCount,bitstream); }
#endif

	/**
	 * Wait for an interrupt from the board.
	 *
	 * @exception mprace::Exception On error.
	 */
	virtual void waitForInterrupt(unsigned int int_id);

	const static unsigned int DESIGN_ID;	// Design ID

	const static unsigned int ISR;	// Interrupt Status Register
	const static unsigned int IER;	// Interrupt Enable Register

	const static unsigned int GER;	// General Error Register
	const static unsigned int GSR;	// General Status Register
	const static unsigned int GCR;	// General Control Register

	const static unsigned int DMA0_BASE;	// Base address of DMA0 channel
	const static unsigned int DMA1_BASE;	// Base address of DMA1 channel

	const static unsigned int MRd_CR;	// MRd Channel Control Register
	const static unsigned int Tx_CR;	// Tx Control Register

	const static unsigned int OUT_FIFO_BASE;	// Base address of Output FIFO Registers
	const static unsigned int IN_FIFO_BASE;		// Base address of Input FIFO Registers

	const static unsigned int ICAP;		// ICAP Port

	const static unsigned int IG_BASE;	// Interrupt Generator base address

        const static unsigned int DMA_TRANS0;
        const static unsigned int DMA_TRANS1;

	const static unsigned int IRQ_CH0;		// Interrupt Source for Channel 0
	const static unsigned int IRQ_CH1;		// Interrupt Source for Channel 1
	const static unsigned int IRQ_IG;		// Interrupt Source for the Interrupt Generator

	const static unsigned int DMA_MEM;		// Perform DMA to Memory
	const static unsigned int DMA_FIFO;		// Perform DMA to FIFO

protected:
	unsigned int *regs;
	unsigned int regs_size;

	unsigned int *mem;
	unsigned int mem_size;

	unsigned int *fifo;
	unsigned int fifo_size;

	DMAEngineWG *dma;

	InterruptGenerator *ig;

	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	ABB(const ABB&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	ABB& operator=(const ABB&) { return *this; };

}; /* class ABB */

} /* namespace mprace */

#endif /*ABB_H_*/
