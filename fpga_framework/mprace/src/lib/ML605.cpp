/*
 * Derived from ML605.cpp, for the Virtex ML605 evaluation kit.
 *
 */
#include "Exception.h"
#include "Board.h"
#include "Logger.h"
#include "ML605.h"
#include "Driver.h"
#include "PCIDriver.h"
#include "DMABuffer.h"
#include "DMAEngineWG.h"
#include "Pin.h"
#include "Register.h"
#include "RegisterTristate.h"

using namespace mprace;


 #include "abb_map.h"
 const unsigned int ML605::DESIGN_ID = (CINT_ADDR_VERSION >> 2);
 const unsigned int ML605::DMA0_BASE = (CINT_ADDR_DMA_DS_PAH >> 2);
 const unsigned int ML605::DMA1_BASE = (CINT_ADDR_DMA_US_PAH >> 2);
 const unsigned int ML605::ISR = (CINT_ADDR_IRQ_STAT >> 2);
 const unsigned int ML605::IER = (0x010 >> 2);

const unsigned int ML605::GER = (0x018 >> 2);
const unsigned int ML605::GSR = (0x020 >> 2);
const unsigned int ML605::GCR = (0x028 >> 2);

const unsigned int ML605::MRd_CR = (0x074 >> 2);
const unsigned int ML605::Tx_CR  = (0x078 >> 2);

const unsigned int ML605::OUT_FIFO_BASE = (0x4010 >> 2);
const unsigned int ML605::IN_FIFO_BASE  = (0x4020 >> 2);

const unsigned int ML605::ICAP = (0x007C >> 2);

const unsigned int ML605::IG_BASE = (0x0080 >> 2);

const unsigned int ML605::DMA_TRANS0 = (0x0094 >> 2);
const unsigned int ML605::DMA_TRANS1 = (0x0098 >> 2);

const unsigned int ML605::IRQ_CH0 = (0);
const unsigned int ML605::IRQ_CH1 = (1);
const unsigned int ML605::IRQ_IG = (2);

const unsigned int ML605::DMA_MEM = (1);
const unsigned int ML605::DMA_FIFO = (2);

ML605::ML605(const unsigned int number) {
	// We need to open the device, map the BARs.

	try {
		// TODO: get the device number from the ML605 board number
		driver = new PCIDriver(number);

		// Open the device
		driver->open();

		// The board has 2 BARs at the moment
		mem = static_cast<unsigned int *>( driver->mmapArea(CINT_BRAM_SPACE_BAR) );
		regs = static_cast<unsigned int *>( driver->mmapArea(CINT_REGS_SPACE_BAR) );
		fifo = static_cast<unsigned int *>( driver->mmapArea(CINT_FIFO_SPACE_BAR) );

		// The driver returns the size in bytes, we convert it to words
		mem_size = (driver->getAreaSize(CINT_BRAM_SPACE_BAR) >> 2);
		regs_size = (driver->getAreaSize(CINT_REGS_SPACE_BAR) >> 2);
		fifo_size = (driver->getAreaSize(CINT_FIFO_SPACE_BAR) >> 2);

		// Initialize the DMAEngine
		dma = new DMAEngineWG( *driver, regs+DMA0_BASE, regs+DMA1_BASE, regs+IER, regs+ISR, regs + DMA_TRANS0, regs + DMA_TRANS1 );

		// Initialize the Interrupt Generator
		ig = new InterruptGenerator( regs+IG_BASE, regs+IER, regs+ISR );
	} catch (std::exception& e) {
		throw;
	} catch (...) {
		throw Exception( Exception::UNKNOWN );
	}

}

ML605::~ML605() {
	// Release DMA Engine structures
	delete dma;

	// Unmap the BARs, close the device
	driver->unmapArea(CINT_REGS_SPACE_BAR);
	driver->unmapArea(CINT_BRAM_SPACE_BAR);
	driver->unmapArea(CINT_FIFO_SPACE_BAR);
	delete driver;
}

void ML605::setReg(const unsigned int address, const unsigned int value) {

	if (address < regs_size)
		*(regs+address) = value;
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->write(address,value);
}

unsigned int ML605::getReg(const unsigned int address) {
	unsigned int value;

	if (address < regs_size)
		value = *(regs+address);
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->read(address,value);

	return value;
}

void ML605::write(const unsigned int address, const unsigned int value) {

	if (address >= mem_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );
	*(mem+address) = value;

	if (log != 0)
		log->write(address,value);

	return;
}

void ML605::writeFIFO(const unsigned int address, const unsigned int value) {
	if (address >= fifo_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );
	*(fifo+address) = value;

	if (log != 0)
		log->write(address,value);

	return;
}


unsigned int ML605::read(const unsigned int address) {
	unsigned int value = 0;

	if (address >= mem_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	value = *(mem+address);

	if (log != 0)
		log->read(address,value);

	return value;
}

unsigned int ML605::readFIFO(const unsigned int address) {
	unsigned int value = 0;

	if (address >= fifo_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	value = *(fifo+address);

	if (log != 0)
		log->read(address,value);

	return value;
}


void ML605::writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i,addr_end;

	// calculate end address
	if (inc)
		addr_end = address+count;
	else
		addr_end = address;

	// Check address range
	if ((address < mem_size) && (addr_end < mem_size)) {
		// write in mem BAR
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		if (inc) {
			for( i=0 ; i<count ; i++ )
				*(mem+address+i) = *(data+i);
		} else {
			for( i=0 ; i<count ; i++ )
				*(mem+address) = *(data+i);
		}
	}
	else if ((address < mem_size+fifo_size) && (addr_end < mem_size+fifo_size)) {
		// write in fifo BAR
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		unsigned int fifo_address = address-mem_size;

		if (inc) {
			for( i=0 ; i<count ; i++ )
				*(mem+fifo_address+i) = *(data+i);
		} else {
			for( i=0 ; i<count ; i++ )
				*(mem+fifo_address) = *(data+i);
		}
	}
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->writeBlock(address,data,count);

}

void ML605::readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i,addr_end;

	// calculate end address
	if (inc)
		addr_end = address+count;
	else
		addr_end = address;

	// Check address range
	if ((address < mem_size) && (addr_end < mem_size)) {
		// read from mem BAR
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		if (inc) {
			for( i=0 ; i<count ; i++ )
				*(data+i) = *(mem+address+i);
		} else {
			for( i=0 ; i<count ; i++ )
				*(data+i) = *(mem+address);
		}
	}
	else if ((address < mem_size+fifo_size) && (addr_end < mem_size+fifo_size)) {
		// read from fifo BAR
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		unsigned int fifo_address = address-mem_size;
		if (inc) {
			for( i=0 ; i<count ; i++ )
				*(data+i) = *(mem+fifo_address+i);
		} else {
			for( i=0 ; i<count ; i++ )
				*(data+i) = *(mem+fifo_address);
		}
	}
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->readBlock(address,data,count);
}

void ML605::writeDMA(const unsigned int address, const DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout)
{
	if (address >= mem_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->host2board(DMA_MEM, address, buf, count, offset, inc, lock);
}

void ML605::writeDMAFIFO(const unsigned int address, const DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout)
{
	if (address >= fifo_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->host2board(DMA_FIFO, address, buf, count, offset, inc, lock);
}

void ML605::readDMA(const unsigned int address, DMABuffer& buf,
		const unsigned int count, const unsigned int offset,
		const bool inc, const bool lock, const float timeout)
{
	if (address >= mem_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->board2host(DMA_MEM, address, buf, count, offset, inc, lock, timeout);
}

void ML605::readDMAFIFO(const unsigned int address, DMABuffer& buf,
		const unsigned int count, const unsigned int offset,
		const bool inc, const bool lock, const float timeout)
{
	if (address >= fifo_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->board2host(DMA_FIFO, address, buf, count, offset, inc, lock, timeout);
}

void ML605::waitForInterrupt(unsigned int int_id) {
	static_cast<PCIDriver*>(driver)->waitForInterrupt(int_id);
}
