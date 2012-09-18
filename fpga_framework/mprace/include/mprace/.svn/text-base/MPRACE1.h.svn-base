#ifndef MPRACE1_H_
#define MPRACE1_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

namespace mprace {

class Board;

/**
 * Implements the library interface for the MPRACE-1. 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-02-12 18:09:18 $
 */
class MPRACE1 : public Board {
public:
	/**
	 * Probes a Driver if it handles an MPRACE1 Board.
	 * @param device The device to probe.
	 * @return true if is an MPRACE1 board, false otherwise.
	 */
	static bool probe(Driver& dev);

	/**
	 * Creates an MPRACE1 board object.
	 * @param number The number of the board to initialize
	 * @todo How are boards enumerated?
	 */
	MPRACE1(const unsigned int number);

	/**
	 * Releases a board.
	 */
	virtual ~MPRACE1();
	
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

protected:
	/* Avoid copy constructor, and copy assignment operator */
	
	/**
	 * Overrides default copy constructor, does nothing.
	 */
	MPRACE1(const MPRACE1&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	MPRACE1& operator=(const MPRACE1&) { return *this; };
}; /* class MPRACE1 */

} /* namespace mprace */

#endif /*MPRACE1_H_*/
