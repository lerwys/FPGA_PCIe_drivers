#ifndef LOGGER_H_
#define LOGGER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

#include <string>
#include <iostream>

namespace mprace {

/**
 * Implements Logging functions for the boards.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-02-12 18:09:16 $
 */
class Logger {
public:
	/**
	 * Create a Logger object.
	 */
	Logger();
	
	/**
	 * Close underlying output streams and destroy the object.
	 */
	~Logger();

	/**
	 * Get the current verbosity level of the logger.
	 * Options include:
	 *  0 - Silent
	 *  1 - Log commands
	 *  2 - Log commands and their debug info
	 */
	int getVerboseLevel() { return verbose; }

	/**
	 * Set the verbosity level of the logger.
	 * Options include:
	 *  0 - Silent
	 *  1 - Log commands
	 *  2 - Log commands and their debug info
	 */
	void setVerboseLevel(const int level) { verbose = level; }

	/**
	 * Insert an entry into the log
	 */
	void logEntry(const std::string& s);

	/**
	 * Insert an entry into the debug log
	 */
	void logDebugEntry(const std::string& s);
	 
	/**
	 * Log a write operation
	 */	
	void write(const unsigned int address, const unsigned int value);

	/**
	 * Log a read operation
	 */	
	void read(const unsigned int address, const unsigned int value);

	/**
	 * Log a writeBlock operation
	 */	
	void writeBlock(const unsigned int address, const unsigned int *startPtr, const unsigned int count);

	/**
	 * Log a readBlock operation
	 */	
	void readBlock(const unsigned int address, const unsigned int *startPtr, const unsigned int count);

protected:
	/**
	 * Output stream to write log entries to.
	 */
	std::ostream *log;
	
	/**
	 * Output stream to write debug information to.
	 */
	std::ostream *debugLog;
	
	/**
	 * Level of verbosity used by the logger.
	 */
	int verbose;

	/**
	 * Open a log file for writing.
	 */	
	void openLog();
	
	/**
	 * Open a debug log file for writing.
	 */	
	void openDebugLog();
		
	
}; /* class Logger */

} /* namespace mprace */

#endif /*LOGGER_H_*/
