/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2009-06-15 11:06:24  mstapelberg
 * fix DMA_MEM = 1 instead of 0, split up into readDMA and readDMAFIFO (similar for write), make testParallelABB -f use the FIFO
 *
 * Revision 1.3  2008-07-17 12:14:11  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.2  2007-03-02 14:58:23  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:13  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "Board.h"
#include "Logger.h"
#include "Exception.h"

using namespace mprace;

void Board::enableLog() {
	log = new Logger();
}

void Board::disableLog() {
	delete log;
	log=0;
}

Board::~Board() {
	if (log != 0)
		delete log;
}

void Board::writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i;
	
	for( i=0 ; i<count ; i++ )
		this->write(address+i,*(data+i));
}

void Board::readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i,j;
	
	for( i=0 ; i<count ; i++ )
		*(data+i) = this->read(address+i);
}

DMAEngine& Board::getDMAEngine() {
	throw Exception( Exception::DMA_NOT_SUPPORTED );
}

void Board::writeDMA(const unsigned int address, const DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout) {
	throw Exception( Exception::DMA_NOT_SUPPORTED );
}

void Board::readDMA(const unsigned int address, DMABuffer& buf, const unsigned
		int count, const unsigned int offset, const bool inc, const
		bool lock, const float timeout) {
	throw Exception( Exception::DMA_NOT_SUPPORTED );
}

void Board::writeDMAFIFO(const unsigned int address, const DMABuffer& buf,
		const unsigned int count, const unsigned int offset, const bool
		inc, const bool lock, const float timeout) {
	throw Exception( Exception::DMA_NOT_SUPPORTED );
}

void Board::readDMAFIFO(const unsigned int address, DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout) {
	throw Exception( Exception::DMA_NOT_SUPPORTED );
}

/**
 *
 * As we are not sure whether the specific board instance has a FIFO, we
 * throw an exception
 *
 */
unsigned Board::readFIFO(unsigned int address) {
	throw Exception( Exception::FIFO_NOT_SUPPORTED );
}

/**
 *
 * As we are not sure whether the specific board instance has a FIFO, we
 * throw an exception
 *
 */
void Board::writeFIFO(unsigned int address, const unsigned int value) {
	throw Exception( Exception::FIFO_NOT_SUPPORTED );
}



Board::ConfigStatus Board::config( const std::string& filename )
{ throw new Exception(Exception::CONFIG_NOT_SUPPORTED); }

Board::ConfigStatus Board::config( const char *filename )
{ throw new Exception(Exception::CONFIG_NOT_SUPPORTED); }

Board::ConfigStatus Board::config( unsigned int byteCount, const char *bitstream )
{ throw new Exception(Exception::CONFIG_NOT_SUPPORTED); }
