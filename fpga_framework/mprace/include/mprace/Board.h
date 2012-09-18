#ifndef BOARD_H_
#define BOARD_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2009-06-15 11:06:24  mstapelberg
 * fix DMA_MEM = 1 instead of 0, split up into readDMA and readDMAFIFO (similar for write), make testParallelABB -f use the FIFO
 *
 * Revision 1.7  2008-07-17 12:14:12  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.6  2008-04-11 09:19:35  marcus
 * Added missing variable initializations. (Thanks, Joern!)
 *
 * Revision 1.5  2008-01-11 10:30:21  marcus
 * Modified Interrupt mechanism. Added an IntSource parameter. This adds experimental support for concurrent interrupts, and handles better some race conditions in the driver.
 *
 * Revision 1.4  2007-10-31 15:48:33  marcus
 * Added IG and KERNEL_PIECES tests.
 * Added Register access functions.
 *
 * Revision 1.3  2007-05-29 07:50:48  marcus
 * Backup commit.
 *
 * Revision 1.2  2007/03/02 14:58:24  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:18  marcus
 * Initial commit.
 *
 *******************************************************************/

#include <string>

namespace mprace {

class Logger;
class Driver;
class DMABuffer;
class DMAEngine;

/**
 * Abstract interface to any board supported by the library. 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.9 $
 * @date    $Date: 2009-06-15 12:29:02 $
 */
class Board {
public:
	/**
	 * Possible States of a Configuration interface
	 */
	enum ConfigStatus { 
		NOT_CONFIGURED, 	//>** FPGA is not configured
		CONFIGURED, 		//>** FPGA was configured successfully.
		CRC_ERROR, 			//>** CRC Error after configuration.
		BUSY 				//>** Device is busy.
	};

	/**
	 * Probes a Driver if it handles this type of Board.
	 * @param device The device to probe.
	 * @return true if is a board of the subclass type, false otherwise.
	 */
	static bool probe(Driver& dev) { return false; }

	/**
	 * Releases a board. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~Board();

	/**
	 * Write a value to a register in the board.
	 * 
	 * @param address Offset (address) in the register space.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void setReg(const unsigned int address, const unsigned int value)=0;
	
	/**
	 * Read a value from a register in the board.
	 * 
	 * @param address Offset (address) in the register space.
	 * @return The value read from the register.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int getReg(const unsigned int address)=0;
	
	/**
	 * Write a value to the board address space.
	 * 
	 * @param address Address in the board.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void write(const unsigned int address, const unsigned int value)=0;

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
	virtual unsigned int read(const unsigned int address)=0;

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
	virtual void writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc = true );

	/**
	 * Read multiple values from the board address space.
	 * 
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @exception mprace::Exception On error.
	 */
	virtual void readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc = true );

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
			0, const bool inc = true, const bool lock=true, const
			float timeout = 0.0 );

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
			offset = 0, const bool inc = true, const bool
			lock=true, const float timeout = 0.0 );

	/**
	 * Read multiple values from the board address space to a DMA Buffer.
	 * 
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords (default = 0).
	 * @param inc Increment the address on the FPGA side (default: true).
	 * @param lock Lock until the write finishes (default: true)
	 * @param timeout The timeout for this DMA read (in miliseconds).
	 * @exception mprace::Exception On error.
	 */
	virtual void readDMA(const unsigned int address, DMABuffer& buf,
			const unsigned int count, const unsigned int offset = 0,
			const bool inc = true, const bool lock = true,
			const float timeout = 0.0);

	/**
	 * Read multiple values from the board's FIFO address space to a DMA Buffer.
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
	 * Enable the logging features.
	 */
	void enableLog();
	
	/**
	 * Disable the logging features.
	 */
	void disableLog();

	/**
	 * Get the Driver for this board.
	 */
	inline Driver& getDriver() { return *driver; }

	/**
	 * Get the DMA Engine for this board.
	 */
	virtual DMAEngine& getDMAEngine();

	/**
	 * Wait for an interrupt from the board.
	 * 
	 * @exception mprace::Exception On error, or if interrupts are not supported.
	 */
	virtual void waitForInterrupt(unsigned int int_param)=0;
	
	/**
	 * Configure the FPGA(s).
	 * 
	 * @param filename Name of the configuration file.
	 * @return ConfigStatus Configuration Status.
	 * @exception mprace::Exception On error.
	 */
	virtual ConfigStatus config( const std::string& filename );

	/**
	 * Configure the FPGA(s).
	 * 
	 * @param filename Name of the configuration file.
	 * @return ConfigStatus Configuration Status.
	 * @exception mprace::Exception On error.
	 */
	virtual ConfigStatus config( const char *filename );

	/**
	 * Configure the FPGA(s).
	 * 
	 * @param byteCount Size of the bitstream, in bytes.
	 * @param bitstream Bytes of the bitstream.
	 * @return ConfigStatus Configuration Status.
	 * @exception mprace::Exception On error.
	 */
	virtual ConfigStatus config( unsigned int byteCount, const char *bitstream );

protected:
	/**
	 * Responsible of logging operations on the board.
	 */
	Logger *log;

	/**
	 * The driver used by the board. Must be initialized by the subclass.
	 */
	Driver *driver;
		
	/**
	 * Creates a board. Protected because only subclasses should 
	 * be instantiated.
	 */
	Board() : log(0), driver(0) {};
	
	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	Board(const Board&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	Board& operator=(const Board&) { return *this; };
}; /* class Board */

} /* namespace mprace */

#endif /*BOARD_H_*/
