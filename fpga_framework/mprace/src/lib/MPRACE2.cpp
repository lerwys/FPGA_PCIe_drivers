/*******************************************************************
 * Change History:
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2009-04-06 13:33:50  marcus
 * Reordered some MPRACE2 registers
 *
 * Revision 1.7  2009-01-15 21:04:00  marcus
 * Sync the repository with the latest version available in mp-pc109
 *
 * Revision 1.5  2008-08-19 08:33:51  marcus
 * Added latest changes, needed for the new smap interface.
 *
 * Revision 1.4  2008-07-17 12:14:11  marcus
 * Added register and changes for test MPRACE2 (debug commit).
 *
 * Revision 1.3  2007-07-06 19:20:36  marcus
 * New Register map for the ABB.
 * User Memory Support with Scatter/Gather Lists.
 *
 * Revision 1.2  2007-03-02 14:58:23  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:15  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "Board.h"
#include "MPRACE2.h"
#include "Exception.h"
#include "Logger.h"
#include "Driver.h"
#include "PCIDriver.h"
#include "DMABuffer.h"
#include "DMAEngineWG.h"
#include "util/Timer.h"

using namespace mprace;
using namespace mprace::util;

#include "mprace2_bridge_map.h"
const unsigned int MPRACE2::DESIGN_ID = (REG_FPGA_VERSION >> 2);
const unsigned int MPRACE2::DMA0_BASE = (REG_DMA_DS_PAH >> 2);
const unsigned int MPRACE2::DMA1_BASE = (REG_DMA_US_PAH >> 2);
const unsigned int MPRACE2::ISR = (REG_INT_STATUS >> 2);

const unsigned int MPRACE2::IER = (REG_INT_ENABLE >> 2);
const unsigned int MPRACE2::GER = (0x018 >> 2);
const unsigned int MPRACE2::GSR = (0x020 >> 2);
const unsigned int MPRACE2::GCR = (0x028 >> 2);

const unsigned int MPRACE2::ICAP = (REG_ICAP_CTRL >> 2);

const unsigned int MPRACE2::IRQ_CH0 = (0);
const unsigned int MPRACE2::IRQ_CH1 = (1);

const unsigned int MPRACE2::MAIN_REGISTER_OFFSET = (0x80000 >> 2);
const unsigned int MPRACE2::MAIN_RESET_LINK = (0xA0 >> 2);
const unsigned int MPRACE2::MAIN_RESET_FIFO = (0xB0 >> 2);
const unsigned int MPRACE2::MAIN_SRC_ADDR = (0xC0 >> 2);
const unsigned int MPRACE2::MAIN_DEST_ADDR = (0xD0 >> 2);
const unsigned int MPRACE2::MAIN_RD_SIZE = (0xF0 >> 2);

const unsigned int MPRACE2::BRIDGE_ERROR_REG = (0x018 >> 2);
const unsigned int MPRACE2::BRIDGE_LINK_REG = (0x074 >> 2);
const unsigned int MPRACE2::BRIDGE_RESET_FIFO = (0x010A);
const unsigned int MPRACE2::BRIDGE_RESET_LINK = (0x040A);
const unsigned int MPRACE2::BRIDGE_RESET_M2BFIFO = (0x080A);

const unsigned int MPRACE2::BRIDGE_TX_RESETFIFO = (0x078 >> 2);

bool MPRACE2::probe(Driver& dev) {
}

MPRACE2::MPRACE2(const unsigned int number) {
	// We need to open the device, map the BARs.

	try {
		// TODO: get the device number from the MPRACE2 board number
		driver = new PCIDriver(number);

		//Open the device
		driver->open();

		// The board has 4 BARs. BAR0 are the bridge registers, BAR1 bridge memory.
		// BAR2 are the Main FPGA registers, BAR3 the main memory.
		bridge_mem = static_cast<unsigned int *>( driver->mmapArea(C_BRAM_BAR) );
		bridge_regs = static_cast<unsigned int *>( driver->mmapArea(C_BREG_BAR) );
		main_mem = static_cast<unsigned int *>( driver->mmapArea(C_MRAM_BAR) );
		main_regs = static_cast<unsigned int *>( driver->mmapArea(C_MREG_BAR) ); // unused

		// The driver returns the size in bytes, we convert it to words
		bridge_mem_size = (driver->getAreaSize(C_BRAM_BAR) >> 2);
		bridge_regs_size = (driver->getAreaSize(C_BREG_BAR) >> 2);
		main_mem_size = (driver->getAreaSize(C_MRAM_BAR) >> 2);
		main_regs_size = (driver->getAreaSize(C_MREG_BAR) >> 2); // unused

		// Initialize the DMAEngine
		dma = new DMAEngineWG( *driver, bridge_regs+DMA0_BASE, bridge_regs+DMA1_BASE, bridge_regs+IER, bridge_regs+ISR, 0, 0 );

		// TODO: Add the config code
	} catch (...) {
		throw Exception( Exception::UNKNOWN );
	}
}

MPRACE2::~MPRACE2() {
	// Release the DMA engine structures
	delete dma;
	// Unmap the BARs, close the device
	driver->unmapArea(0);
	driver->unmapArea(1);
	driver->unmapArea(2);
	driver->unmapArea(3);
	delete driver;
}

void MPRACE2::resetMGTlink() {

	// Should wait for an event isntead of use timers ... but for now, we just wait some large time (1s)
	// Reset bridge MGT Link
	this->setBridgeReg( BRIDGE_LINK_REG, BRIDGE_RESET_LINK );
	Timer::wait(1.0);

	// Reset main MGT Link
	this->setReg( MAIN_RESET_LINK, 0xFFFFFFFF );
	Timer::wait(1.0);
}

void MPRACE2::setReg(const unsigned int address, const unsigned int value) {

	if (MAIN_REGISTER_OFFSET+address < main_mem_size)
		*(main_mem+MAIN_REGISTER_OFFSET+address) = value;
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->write(address,value);
}

unsigned int MPRACE2::getReg(const unsigned int address) {
	unsigned int value;

	if (MAIN_REGISTER_OFFSET+address < main_mem_size) { 
#ifndef MAIN_LOOPBACK
		// Signal the main FPGA to start reading, before telling the DMA Engine what to do.
		this->setReg(MAIN_SRC_ADDR, (MAIN_REGISTER_OFFSET+address)<<2);
		this->setReg(MAIN_DEST_ADDR, (MAIN_REGISTER_OFFSET+address)<<2);
		this->setReg(MAIN_RD_SIZE, 1);
#endif
		value = *(main_mem+MAIN_REGISTER_OFFSET+address);
	} else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->read(address,value);

	return value;
}

void MPRACE2::setBridgeReg(const unsigned int address, const unsigned int value) {

	if (address < bridge_regs_size)
		*(bridge_regs+address) = value;
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->write(address,value);
}

unsigned int MPRACE2::getBridgeReg(const unsigned int address) {
	unsigned int value;

	if (address < bridge_regs_size)
		value = *(bridge_regs+address);
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->read(address,value);

	return value;
}


void MPRACE2::write(unsigned int address, unsigned int value) {

	if (address < main_mem_size)
		*(main_mem+address) = value;
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->write(address,value);

	return;
}

unsigned int MPRACE2::read(unsigned int address) {
	unsigned int value;

	if (address < main_mem_size) {
#ifndef MAIN_LOOPBACK
		this->setReg(MAIN_SRC_ADDR, address<<2);
		this->setReg(MAIN_DEST_ADDR, (address<<2));
		this->setReg(MAIN_RD_SIZE, 1);
#endif
		value = *(main_mem+address);
	} else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->read(address,value);

	return value;
}

void MPRACE2::writeBlock(const unsigned int address, const unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i,j;

	// Check address range
	if ((address <= main_mem_size) && ((address+count-1) < main_mem_size)) {
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		if (inc) {
			for( i=0; i<count ; i++ )
				*(main_mem+address+i) = *(data+i);
		} else {
			for( i=0; i<count ; i++ ) {
				*(main_mem+address) = *(data+i);
			}
		}
	}
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->writeBlock(address,data,count);

}

void MPRACE2::readBlock(const unsigned int address, unsigned int *data, const unsigned int count, const bool inc) {
	unsigned int i,j;

	if ((address <= main_mem_size) && ((address+count-1) < main_mem_size)) {
		/* for performance, we take the comparison out of the loop,
		 * and repeat the code.
		 */
		if (inc) {

#ifndef MAIN_LOOPBACK
			this->setReg(MAIN_SRC_ADDR, address<<2);
			this->setReg(MAIN_DEST_ADDR, (address<<2));
			this->setReg(MAIN_RD_SIZE, count);
#endif
			for( i=0 ; i<count ; i++ )
				*(data+i) = *(main_mem+address+i);
		} else {
			for( i=0 ; i<count ; i++ ) {
#ifndef MAIN_LOOPBACK
				this->setReg(MAIN_SRC_ADDR, address<<2);
				this->setReg(MAIN_DEST_ADDR, (address<<2));
				this->setReg(MAIN_RD_SIZE, 1);
#endif
				*(data+i) = *(main_mem+address);
			}
		}
	}
	else
		throw Exception( Exception::ADDRESS_OUT_OF_RANGE );

	if (log != 0)
		log->readBlock(address,data,count);
}

void MPRACE2::writeDMA(const unsigned int address, const DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc, const bool lock, const float timeout ) {
	dma->host2board(C_MRAM_BAR,address,buf,count,offset,inc,lock,timeout);
}

void MPRACE2::readDMA(const unsigned int address, DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc, const bool lock, const float timeout ) {

#ifndef MAIN_LOOPBACK
	// Signal the main FPGA to start reading, before telling the DMA Engine what to do.
	this->setReg(MAIN_SRC_ADDR, (address<<2));
	this->setReg(MAIN_DEST_ADDR, (address<<2));
	this->setReg(MAIN_RD_SIZE, count);
#endif

	dma->board2host(C_MRAM_BAR,address,buf,count,offset,inc,lock,timeout);
}

void MPRACE2::waitForInterrupt(unsigned int int_id) {
	static_cast<PCIDriver*>(driver)->waitForInterrupt(int_id);
}
