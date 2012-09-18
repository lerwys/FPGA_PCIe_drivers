#ifndef MPRACE2_H_
#define MPRACE2_H_

/*******************************************************************
 * Change History:
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2008-07-17 12:14:12  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.2  2007-03-02 14:58:25  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:17  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "Board.h"
#include "DMAEngineWG.h"
#include "Exception.h"

namespace mprace {

class DMAEngineWG;

/**
 * Implements the library interface for the MPRACE-2.
 * The address space provides access to the Main FPGA only. All other
 * memory areas must be accessed thru the specific device interface
 * associated with it.
 *
 * @author  Guillermo Marcus
 * @version $Revision: 1.4 $
 * @date    $Date: 2009-04-06 13:33:50 $
 */
class MPRACE2 : public mprace::Board {
public:
	/**
	 * Probes a Driver if it handles an MPRACE2 Board.
	 * @param device The device to probe.
	 * @return true if is an MPRACE2 board, false otherwise.
	 */
	static bool probe(Driver& dev);

	/**
	 * Creates an MPRACE2 board object.
	 * @param number The number of the board to initialize
	 * @todo How are boards enumerated?
	 */
	MPRACE2(const unsigned int number);

	/**
	 * Releases a board.
	 */
	virtual ~MPRACE2();

	/**
	 * Write a value to a register in the main FPGA.
	 *
	 * @param address Offset (address) in the register space.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	void setReg(const unsigned int address, const unsigned int value);

	/**
	 * Read a value from a register in the main FPGA.
	 *
	 * @param address Offset (address) in the register space.
	 * @return The value read from the register.
	 * @exception mprace::Exception On error.
	 */
	unsigned int getReg(const unsigned int address);

	/**
	 * Write a value to a register in the bridge FPGA.
	 *
	 * @param address Offset (address) in the register space.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	void setBridgeReg(const unsigned int address, const unsigned int value);

	/**
	 * Read a value from a register in the bridge FPGA.
	 *
	 * @param address Offset (address) in the register space.
	 * @return The value read from the register.
	 * @exception mprace::Exception On error.
	 */
	unsigned int getBridgeReg(const unsigned int address);

	/**
	 * Write a value to the board address space.
	 *
	 * @param address Address in the board.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	void write(unsigned int address, unsigned int value);

	/**
	 * Read a value from the board address space.
	 *
	 * @param address Address in the board.
	 * @return The value read from the board.
	 * @exception mprace::Exception On error.
	 */
	unsigned int read(unsigned int address);

	/**
	 * Write multiple values to the board address space.
	 *
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @exception mprace::Exception On error.
	 */
	void writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc = true );

	/**
	 * Read multiple values from the board address space.
	 *
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @exception mprace::Exception On error.
	 */
	void readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc = true );

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
	void writeDMA(const unsigned int address, const DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc = true, const bool lock=true, const float timeout = 0.0 );

	/**
	 * Read multiple values from the board address space to a DMA Buffer.
	 *
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @exception mprace::Exception On error.
	 */
	void readDMA(const unsigned int address, DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc = true, const bool lock=true, const float timeout = 0.0 );

	/**
	 * Get the DMA Engine for this board.
	 */
	virtual DMAEngine& getDMAEngine() { return *dma; }

	/**
	 * Reset the MGT link between the Master and the Bridge FPGAs
	 */
	void resetMGTlink();

	/**
	 * Wait for an interrupt from the board.
	 *
	 * @exception mprace::Exception On error, or if interrupts are not supported.
	 */
	virtual void waitForInterrupt(unsigned int int_param);

	/**
	 * Configure the Main FPGA.
	 *
	 * @param filename File name of the configuration file. Can be in BIT, BIN or RBT format.
	 * @return Board:ConfigStatus Configuration Status
	 * @exception mprace::Exception On error.
	 */
	virtual inline mprace::Board::ConfigStatus config( const std::string& filename )
	{ throw mprace::Exception( mprace::Exception::UNKNOWN); }
//	{ return v4config->config(filename); }

	/**
	 * Configure the Main FPGA.
	 *
	 * @param filename File name of the configuration file. Can be in BIT, BIN or RBT format.
	 * @return Board:ConfigStatus Configuration Status
	 * @exception mprace::Exception On error.
	 */
	virtual inline mprace::Board::ConfigStatus config( const char *filename )
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
	virtual inline mprace::Board::ConfigStatus config( unsigned int byteCount, const char *bitstream )
	{ throw mprace::Exception( mprace::Exception::UNKNOWN); }
//	{ return v4config->config(byteCount,bitstream); }

	const static unsigned int DESIGN_ID;	// Design ID

	const static unsigned int ISR;	// Interrupt Status Register
	const static unsigned int IER;	// Interrupt Enable Register

	const static unsigned int GER;	// General Error Register
	const static unsigned int GSR;	// General Status Register
	const static unsigned int GCR;	// General Control Register

	const static unsigned int DMA0_BASE;	// Base address of DMA0 channel
	const static unsigned int DMA1_BASE;	// Base address of DMA1 channel

	const static unsigned int ICAP;		// ICAP Port

	const static unsigned int IRQ_CH0;		// Interrupt Source for Channel 0
	const static unsigned int IRQ_CH1;		// Interrupt Source for Channel 1

	const static unsigned int MAIN_REGISTER_OFFSET;		// Main FPGA Register offset (registers are now mapped in the data area)

	const static unsigned int MAIN_SRC_ADDR;		// Main FPGA Source Address read
	const static unsigned int MAIN_DEST_ADDR;		// Main FPGA Destination Address read
	const static unsigned int MAIN_RD_SIZE;			// Main FPGA read size

	const static unsigned int MAIN_RESET_LINK;		// Reset MGT Link (master)
	const static unsigned int MAIN_RESET_FIFO;		// Reset MGT Link FIFO (master)

	const static unsigned int BRIDGE_TX_RESETFIFO;	// TX Reset FIFO register
	const static unsigned int BRIDGE_LINK_REG;		// Reset MGT Register (bridge)
	const static unsigned int BRIDGE_RESET_FIFO;		// Reset MGT FIFO (bridge)
	const static unsigned int BRIDGE_RESET_LINK;		// Reset MGT Link (bridge)
	const static unsigned int BRIDGE_RESET_M2BFIFO;		// Reset M2B FIFO (bridge)

	const static unsigned int BRIDGE_ERROR_REG;		// General Error Register

protected:
	unsigned int *bridge_mem;
	unsigned int bridge_mem_size;	/** Size of the Bridge Memory area, in 32-bit words */

	unsigned int *bridge_regs;
	unsigned int bridge_regs_size;	/** Size of the Bridge Register area, in 32-bit words */

	unsigned int *main_mem;
	unsigned int main_mem_size;			/** Size of the Main Memory area, in 32-bit words */

	unsigned int *main_regs;
	unsigned int main_regs_size;		/** Size of the Main Register area, in 32-bit words */

	DMAEngineWG *dma;			/** The DMAEngine of this board */

	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	MPRACE2(const MPRACE2&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	MPRACE2& operator=(const MPRACE2&) { return *this; };
}; /* class MPRACE2 */

} /* namespace mprace */

#endif /*MPRACE2_H_*/
