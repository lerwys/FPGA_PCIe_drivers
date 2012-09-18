#ifndef MPRACE_EXCEPTION_H_
#define MPRACE_EXCEPTION_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2008-07-17 12:14:12  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.4  2008-01-11 10:29:09  marcus
 * Modified ifdef to avoid name clash with pciDriver Exception.
 *
 * Revision 1.3  2007-07-17 10:35:19  marcus
 * Added an optional parameter to the exception
 *
 * Revision 1.2  2007-05-29 07:50:46  marcus
 * Backup commit.
 *
 * Revision 1.1  2007/02/12 18:09:16  marcus
 * Initial commit.
 *
 *******************************************************************/

#include <exception>
#include <string>

namespace mprace {

/**
 * Represents any possible error inside the library.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.6 $
 * @date    $Date: 2009-06-15 12:29:02 $
 */
class Exception : public std::exception {
public:

	/**
	 * Possible Exception types.
	 */
	enum Types {
	 	UNKNOWN,				//**< Unknown exception.
	 	KERNEL_ALLOC_FAILED,	//**< Kernel allocation failed.
	 	USER_MMAP_FAILED,		//**< UserMemory mmap failed.
	 	INTERRUPT_FAILED,		//**< Interrupt failed.
	 	NOT_OPEN,				//**< Device not open.
	 	FILE_NOT_FOUND,			//**< File Not Found.
	 	UNKNOWN_FILE_FORMAT,	//**< Unknown File Format.
	 	DMA_NOT_SUPPORTED,		//**< DMA is not supported.
	 	CONFIG_NOT_SUPPORTED,	//**< FPGA config is not supported.
	 	INVALID_FPGA_NUMBER,	//**< Invalid FPGA number.
	 	ADDRESS_OUT_OF_RANGE,	//**< Address out of range.
	 	HUGEPAGES_OPEN_FAILED,	//**< Huge Pages open failed.
	 	HUGEPAGES_MMAP_FAILED,	//**< Huge Pages mmap failed.
		FIFO_NOT_SUPPORTED,
		DMA_TIMEOUT,
		EMPTY_TRANSFER,
		OVERSIZED_TRANSFER
	 };
	
	const static char* descriptions[];
	
	
	/**
	 * Create an Exception object.
	 * 
	 * @param t The exception type.
	 */
	Exception(Types t) : type(t), param(0) {}

	/**
	 * Create an Exception object with a parameter.
	 * 
	 * @param t The exception type.
	 * @param p The exception parameter.
	 */
	Exception(Types t, int p) : type(t), param(p) {}
	
	/**
	 * Get the type of the exception
	 * @return The type of the exception
	 */
	Types getType() { return type; }

	/**
	 * Get the parameter of the exception
	 * @return The parameter passed to the exception on creation
	 */
	int getParameter() { return param; }
	
	/**
	 * Returns a string describing the exception
	 */
	 virtual const char* what() const throw() {
	 	return descriptions[type];
	 }
	
private:
	/**
	 * The type of the current exception object.
	 */
	Types type;
	int param;
}; /* class Exception */

} /* namespace mprace */

#endif /*MPRACE_EXCEPTION_H_*/
