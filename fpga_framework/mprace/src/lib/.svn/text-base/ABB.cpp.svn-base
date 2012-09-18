/*******************************************************************
 * Change History:
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2009-06-18 09:59:37  mstapelberg
 * Correct address checking for the FIFO, modify testParallelABB to write to the correct address / size
 *
 * Revision 1.9  2009-06-15 12:29:02  mstapelberg
 * Split up read/write for PIO into the FIFO aswell, adapt testcase
 *
 * Revision 1.8  2009-06-15 11:06:24  mstapelberg
 * fix DMA_MEM = 1 instead of 0, split up into readDMA and readDMAFIFO (similar for write), make testParallelABB -f use the FIFO
 *
 * Revision 1.7  2009-04-17 13:51:31  marcus
 * + Added multiple PCI_BAR DMA support, needed for the new ABB FIFO tests.
 * + Added MAIN_LOOPBACK flags, in order to enable/disable the neccesary changes for loopback tests.
 *
 * Revision 1.6  2008-01-11 10:31:15  marcus
 * Modified Interrupt mechanism. Added an IntSource parameter. This adds experimental support for concurrent interrupts, and handles better some race conditions in the driver.
 *
 * Revision 1.5  2007-10-31 15:49:47  marcus
 * Added IG and KERNEL_PIECES tests.
 * Added Register access functions.
 *
 * Revision 1.4  2007-07-06 19:20:33  marcus
 * New Register map for the ABB.
 * User Memory Support with Scatter/Gather Lists.
 *
 * Revision 1.3  2007-05-29 07:50:51  marcus
 * Backup commit.
 *
 * Revision 1.2  2007/03/02 14:58:22  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:14  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "Exception.h"
#include "Board.h"
#include "Logger.h"
#include "ABB.h"
#include "Driver.h"
#include "PCIDriver.h"
#include "DMABuffer.h"
#include "DMAEngineWG.h"
#include "Pin.h"
#include "Register.h"
#include "RegisterTristate.h"
#include "PinInRegister.h"

using namespace mprace;


#ifdef OLD_REGISTERS
 const unsigned int ABB::DESIGN_ID = (0x0);
 const unsigned int ABB::DMA0_BASE = (0x0C040 >> 2);
 const unsigned int ABB::DMA1_BASE = (0x0C000 >> 2);
 const unsigned int ABB::ISR = (0x0);
 const unsigned int ABB::IER = (0x0);
#else
 #include "abb_map.h"
 const unsigned int ABB::DESIGN_ID = (CINT_ADDR_VERSION >> 2);
 const unsigned int ABB::DMA0_BASE = (CINT_ADDR_DMA_DS_PAH >> 2);
 const unsigned int ABB::DMA1_BASE = (CINT_ADDR_DMA_US_PAH >> 2);
 const unsigned int ABB::ISR = (CINT_ADDR_IRQ_STAT >> 2);
 const unsigned int ABB::IER = (0x010 >> 2);
#endif

const unsigned int ABB::GER = (0x018 >> 2);
const unsigned int ABB::GSR = (0x020 >> 2);
const unsigned int ABB::GCR = (0x028 >> 2);

const unsigned int ABB::MRd_CR = (0x074 >> 2);
const unsigned int ABB::Tx_CR  = (0x078 >> 2);

const unsigned int ABB::OUT_FIFO_BASE = (0x4010 >> 2);
const unsigned int ABB::IN_FIFO_BASE  = (0x4020 >> 2);

const unsigned int ABB::ICAP = (0x007C >> 2);

const unsigned int ABB::IG_BASE = (0x0080 >> 2);

const unsigned int ABB::DMA_TRANS0 = (0x0094 >> 2);
const unsigned int ABB::DMA_TRANS1 = (0x0098 >> 2);

const unsigned int ABB::IRQ_CH0 = (0);
const unsigned int ABB::IRQ_CH1 = (1);
const unsigned int ABB::IRQ_IG = (2);

const unsigned int ABB::DMA_MEM = (1);
const unsigned int ABB::DMA_FIFO = (2);

ABB::ABB(const unsigned int number) {
	// We need to open the device, map the BARs.

	try {
		// TODO: get the device number from the ABB board number
		driver = new PCIDriver(number);

		// Open the device
		driver->open();

		// The board has 2 BARs at the moment
#ifdef OLD_REGISTERS
		mem = static_cast<unsigned int *>( driver->mmapArea(0) );
		regs = static_cast<unsigned int *>( driver->mmapArea(1) );

		// The driver returns the size in bytes, we convert it to words
		mem_size = (driver->getAreaSize(0) >> 2);
		regs_size = (driver->getAreaSize(1) >> 2);
#else
		mem = static_cast<unsigned int *>( driver->mmapArea(CINT_BRAM_SPACE_BAR) );
		regs = static_cast<unsigned int *>( driver->mmapArea(CINT_REGS_SPACE_BAR) );
		fifo = static_cast<unsigned int *>( driver->mmapArea(CINT_FIFO_SPACE_BAR) );

		// The driver returns the size in bytes, we convert it to words
		mem_size = (driver->getAreaSize(CINT_BRAM_SPACE_BAR) >> 2);
		regs_size = (driver->getAreaSize(CINT_REGS_SPACE_BAR) >> 2);
		fifo_size = (driver->getAreaSize(CINT_FIFO_SPACE_BAR) >> 2);
#endif

		// Initialize the DMAEngine
		dma = new DMAEngineWG( *driver, regs+DMA0_BASE, regs+DMA1_BASE, regs+IER, regs+ISR, regs + DMA_TRANS0, regs + DMA_TRANS1 );
		// Set the loop limit for the waitChannel.
		// 1100 seems to be a nice working value for ABB boards.
		dma->setLoopLimit(1100);

		// Initialize the Interrupt Generator
		ig = new InterruptGenerator( regs+IG_BASE, regs+IER, regs+ISR );

#if 0
		// This does not belong to the ABB
		// Initialize the Configuration interface
		bio_reg = new RegisterTristate( regs+BIO_BASE+0, regs+BIO_BASE+2, regs+BIO_BASE+1 );
		ctl_reg = new RegisterTristate( regs+CTL_BASE+0, regs+CTL_BASE+2, regs+CTL_BASE+1 );
		smap_reg = new RegisterTristate( regs+SMAP_BASE+0, regs+SMAP_BASE+2, regs+SMAP_BASE+1 );
		PinInRegister smap_mode_bit( *bio_reg, 0 );
		PinInRegister smap_mode_n_bit( *bio_reg, 1 );

		// set to SMAP8
		smap_mode_bit.set();
		smap_mode_n_bit.clear();

		v4config = new V4ConfigSMAP(
							smap_reg,
							V4ConfigSMAP::SMAP8,
							new PinInRegister( *ctl_reg, 17 ),
							new PinInRegister( *ctl_reg, 16 ),
							new PinInRegister( *ctl_reg, 20 ),
							new PinInRegister( *ctl_reg, 19 ),
							new PinInRegister( *ctl_reg, 18 ),
							new PinInRegister( *ctl_reg, 21 )
						);
#endif

	} catch (std::exception& e) {
		throw;
	} catch (...) {
		throw Exception( Exception::UNKNOWN );
	}

}

ABB::~ABB() {
	// Release DMA Engine structures
	delete dma;

	// Unmap the BARs, close the device
	driver->unmapArea(CINT_REGS_SPACE_BAR);
	driver->unmapArea(CINT_BRAM_SPACE_BAR);
	driver->unmapArea(CINT_FIFO_SPACE_BAR);
	delete driver;
}

void ABB::setReg(const unsigned int address, const unsigned int value) {

	if (address < regs_size)
		*(regs+address) = value;
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->write(address,value);
}

unsigned int ABB::getReg(const unsigned int address) {
	unsigned int value;

	if (address < regs_size)
		value = *(regs+address);
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->read(address,value);

	return value;
}

void ABB::write(const unsigned int address, const unsigned int value) {

	if (address >= mem_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );
	*(mem+address) = value;

	if (log != 0)
		log->write(address,value);

	return;
}

void ABB::writeFIFO(const unsigned int address, const unsigned int value) {
	if (address >= fifo_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );
	*(fifo+address) = value;

	if (log != 0)
		log->write(address,value);

	return;
}


unsigned int ABB::read(const unsigned int address) {
	unsigned int value = 0;

	if (address >= mem_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	value = *(mem+address);

	if (log != 0)
		log->read(address,value);

	return value;
}

unsigned int ABB::readFIFO(const unsigned int address) {
	unsigned int value = 0;

	if (address >= fifo_size)
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	value = *(fifo+address);

	if (log != 0)
		log->read(address,value);

	return value;
}


void ABB::writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc) {
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

void ABB::readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc) {
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

void ABB::writeDMA(const unsigned int address, const DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout)
{
	if (address >= mem_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	if (inc) {
		if (count > 8192)
			throw Exception(Exception::OVERSIZED_TRANSFER);
	}

	dma->host2board(DMA_MEM, address, buf, count, offset, inc, lock);
}

void ABB::writeDMAFIFO(const unsigned int address, const DMABuffer& buf, const
		unsigned int count, const unsigned int offset, const bool inc,
		const bool lock, const float timeout)
{
	if (address >= fifo_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->host2board(DMA_FIFO, address, buf, count, offset, inc, lock);
}

void ABB::readDMA(const unsigned int address, DMABuffer& buf,
		const unsigned int count, const unsigned int offset,
		const bool inc, const bool lock, const float timeout)
{
	if (address >= mem_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	if (inc) {
		if (count > 8192)
			throw Exception(Exception::OVERSIZED_TRANSFER);
	}

	dma->board2host(DMA_MEM, address, buf, count, offset, inc, lock, timeout);
}

void ABB::readDMAFIFO(const unsigned int address, DMABuffer& buf,
		const unsigned int count, const unsigned int offset,
		const bool inc, const bool lock, const float timeout)
{
	if (address >= fifo_size)
		throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	dma->board2host(DMA_FIFO, address, buf, count, offset, inc, lock, timeout);
}

void ABB::waitForInterrupt(unsigned int int_id) {
	static_cast<PCIDriver*>(driver)->waitForInterrupt(int_id);
}
