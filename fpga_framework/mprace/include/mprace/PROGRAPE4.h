#ifndef PROGRAPE4_H_
#define PROGRAPE4_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

namespace mprace {

class Board;

/**
 * Implements the library interface for the PROGRAPE4.
 * The address space provides access to the Local Bus space only. All other
 * memory areas must be accessed thru the specific device interface
 * associated with it.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-02-12 18:09:16 $
 */
class PROGRAPE4 : public Board {
public:
	/**
	 * Probes a Driver if it handles an PROGRAPE4 Board.
	 * @param device The device to probe.
	 * @return true if is an PROGRAPE4 board, false otherwise.
	 */
	static bool probe(Driver& dev);

	/**
	 * Creates an PROGRAPE4 board object.
	 * @param number The number of the board to initialize
	 * @todo How are boards enumerated?
	 */
	MPRACE2(const unsigned int number);

	/**
	 * Releases a board.
	 */
	virtual ~MPRACE2();
	
	/**
	 * Write a value to the board address space.
	 * 
	 * @param address Address in the board.
	 * @param value Value to write.
	 * @exception mprace::Exception On error.
	 */
	virtual void write(unsigned int address, unsigned int value);
	
	/**
	 * Read a value from the board address space.
	 * 
	 * @param address Address in the board.
	 * @return The value read from the board.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int read(unsigned int address);

	/**
	 * Write multiple values to the board address space.
	 * 
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @exception mprace::Exception On error.
	 */
	virtual void writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count);

	/**
	 * Read multiple values from the board address space.
	 * 
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @exception mprace::Exception On error.
	 */
	virtual void readBlock(const unsigned int address, unsigned int *data, const unsigned int count);

	/**
	 * Write multiple values from a DMA Buffer to the board address space.
	 * 
	 * @param address Address in the board.
	 * @param data Values to write.
	 * @param count Number of values to write, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords.
	 * @exception mprace::Exception On error.
	 */
	virtual void writeDMA(const unsigned int address, const DMABuffer& buf, const unsigned int count, const unsigned int offset);

	/**
	 * Read multiple values from the board address space to a DMA Buffer.
	 * 
	 * @param address Address in the board.
	 * @param data An array where to read values into.
	 * @param count Number of values to read from the board's address space, in dwords.
	 * @param offset Offset in the DMA buffer, in dwords.
	 * @exception mprace::Exception On error.
	 */
	virtual void readDMA(const unsigned int address, DMABuffer& buf, const unsigned int count, const unsigned int offset);

protected:
	unsigned int *bridge;
	unsigned int bridge_size;
	
	unsigned int *main;
	unsigned int main_size;


	/* Avoid copy constructor, and copy assignment operator */
	
	/**
	 * Overrides default copy constructor, does nothing.
	 */
	PROGRAPE4(const PROGRAPE4&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	PROGRAPE4& operator=(const PROGRAPE4&) { return *this; };
}; /* class PROGRAPE4 */

} /* namespace mprace */

#endif /*PROGRAPE4_H_*/
