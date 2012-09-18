/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2008-03-20 13:26:56  marcus
 * Added support for DMA descriptors to be in user memory.
 *
 * Revision 1.4  2008-01-11 10:31:15  marcus
 * Modified Interrupt mechanism. Added an IntSource parameter. This adds experimental support for concurrent interrupts, and handles better some race conditions in the driver.
 *
 * Revision 1.3  2007-05-29 07:50:52  marcus
 * Backup commit.
 *
 * Revision 1.2  2007/03/02 14:58:24  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:15  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "Driver.h"
#include "PCIDriver.h"
#include "Exception.h"
#include "pciDriver/lib/pciDriver.h"
#include <time.h>
#include <iostream>

using namespace mprace;

const unsigned int PCIDriver::max_retries = 20;
const unsigned long PCIDriver::retry_sleep = 10000000L;	// in nanoseconds 

PCIDriver::PCIDriver(const unsigned int num) {
	dev = new pciDriver::PciDevice(num);
	retry_stats_kernel = new unsigned long[ PCIDriver::max_retries ];
	for(int i=0;i<max_retries;++i)
		retry_stats_kernel[i]=0UL;
	
	//Init bars (Joern)
	for(int i=0; i<6;++i)
		bar[i] = 0;
}

PCIDriver::~PCIDriver() {
	delete retry_stats_kernel;
	delete dev;
}

void PCIDriver::open() {
	dev->open();
}

void PCIDriver::close() {
	dev->close();
}

void *PCIDriver::mmapArea(const unsigned int num) {
	if (bar[num] == 0) {
		bar[num] = dev->mapBAR(num);
	}
	return bar[num];
}

void PCIDriver::unmapArea(const unsigned int num) {
	if (bar[num] != 0) {
		dev->unmapBAR(num,bar[num]);
		bar[num]=0;
	}
}

unsigned int PCIDriver::getAreaSize(const unsigned int num) {
	return dev->getBARsize(num);
}

pciDriver::KernelMemory& PCIDriver::allocKernelMemory(const unsigned int size ) {
	
	int retryCount=0;
	struct timespec ns,rem;
	ns.tv_sec = 0L;
	ns.tv_nsec = PCIDriver::retry_sleep;

	while (retryCount < PCIDriver::max_retries) {
		++(this->retry_stats_kernel[ retryCount ]);
		
		try {
			return dev->allocKernelMemory(size);
		} catch ( pciDriver::Exception& e) {
			if (e.getType() == pciDriver::Exception::ALLOC_FAILED) {
				(this->retry_stats_kernel[ retryCount ])--;				
				++retryCount;
			} else
				throw e;
		} catch (...) {
			throw mprace::Exception( mprace::Exception::UNKNOWN );
		}
	
		nanosleep(&ns,&rem);
	}

	std::cout << "Kernel Alloc Retry Stats: " << std::endl;
	for(int i=0;i<PCIDriver::max_retries;++i)
		std::cout << "[" << i << "]: " << this->retry_stats_kernel[ i ] << std::endl;
	std::cout << std::endl;

	throw mprace::Exception( mprace::Exception::KERNEL_ALLOC_FAILED );

}

pciDriver::UserMemory& PCIDriver::mapUserMemory( void *mem, unsigned int size, bool merged ) {
	try {
		return dev->mapUserMemory(mem,size,merged);
	} catch ( pciDriver::Exception& e) {
		if (e.getType() == pciDriver::Exception::SGMAP_FAILED)
			throw mprace::Exception( mprace::Exception::USER_MMAP_FAILED );
		else
			throw e;
	} catch (...) {
		throw mprace::Exception( mprace::Exception::UNKNOWN );
	}
}

void PCIDriver::waitForInterrupt(unsigned int int_id) {
	try {
		dev->waitForInterrupt(int_id);
	} catch ( pciDriver::Exception& e) {
		if (e.getType() == pciDriver::Exception::NOT_OPEN)
			throw mprace::Exception( mprace::Exception::NOT_OPEN );
		else if (e.getType() == pciDriver::Exception::INTERRUPT_FAILED)
			throw mprace::Exception( mprace::Exception::INTERRUPT_FAILED );
		else
			throw e;
	} catch (...) {
		throw mprace::Exception( mprace::Exception::UNKNOWN );
	}
}
