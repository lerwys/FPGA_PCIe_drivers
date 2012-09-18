#ifndef PCIDRIVER_H_
#define PCIDRIVER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2008-01-11 10:30:21  marcus
 * Modified Interrupt mechanism. Added an IntSource parameter. This adds experimental support for concurrent interrupts, and handles better some race conditions in the driver.
 *
 * Revision 1.3  2007-05-29 07:50:45  marcus
 * Backup commit.
 *
 * Revision 1.2  2007/03/02 14:58:25  marcus
 * DMAEngineWG basic functionality working.
 *
 * Revision 1.1  2007/02/12 18:09:16  marcus
 * Initial commit.
 *
 *******************************************************************/

namespace pciDriver {
	class PciDevice;
	class KernelMemory;
	class UserMemory;
}

namespace mprace {

class Driver;

/**
 * Interface between the mprace library and the new PCIDriver.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.5 $
 * @date    $Date: 2008-03-20 13:26:56 $
 */
class PCIDriver : public Driver {
public:
	PCIDriver(const unsigned int num);
	virtual ~PCIDriver();
	
	/**
	 * Open the device.
	 */
	void open();

	/**
	 * Close the device.
	 */
	void close();

	/**
	 * Memory Map a BAR into User Space Memory.
	 * @param num the area number to mmap. BAR0-5.
	 * @return A pointer to access the requested area.
	 * @exception mprace::Exception on Error.
	 */
	void *mmapArea(const unsigned int num);
	
	/**
	 * Release an area previously mapped into User Space Memory.
	 * @param num the area number to mmap. BAR0-5.
	 * @exception mprace::Exception on Error.
	 */
	void unmapArea(const unsigned int num);
	
	/**
	 * Get the size of the area.
	 * @param num the area number to mmap. BAR0-5.
	 * @return the size of the area, in bytes.
	 */
	unsigned int getAreaSize(const unsigned int num);

	/**
	 * Allocate Kernel memory, and return an underlying object describing it.
	 * @param size Size of the request area, in bytes
	 * @return A pciDriver::KernelMemory object.
	 * @exception mprace::Exception on Error.
	 */
	pciDriver::KernelMemory& allocKernelMemory(const unsigned int size );
	
	/**
	 * Lock user memory, create a SG list for it, and return an underlying object describing it.
	 * @param mem A pointer to the memory area to map.
	 * @param size Size of the request area, in bytes.
	 * @param merged Request descriptors to be merged if pages are consecutive blocks.
	 * @return A pciDriver::KernelMemory object.
	 * @exception mprace::Exception on Error.
	 */
	pciDriver::UserMemory& mapUserMemory( void *mem, unsigned int size, bool merged = true );

	/**
	 * Wait for an Interrupt to arrive.
	 * This is a blocking call.
	 * @param int_id The ID value of the interrupt to wait for. This is driver dependent.
	 */
	void waitForInterrupt(unsigned int int_id);
	
protected:
	/**
	 * The underlying PciDevice represented by this object.
	 */
	 pciDriver::PciDevice *dev;
	 
	/**
	 * Cache the mmap of the BARs. We will map them only once
	 * at the opening of the device.
	 */
	void *bar[6];

	static const unsigned int max_retries;
	static const unsigned long retry_sleep;
	
	unsigned long *retry_stats_kernel;
	
}; /* class PCIDriver */

} /* namespace mprace */

#endif /*PCIDRIVER_H_*/
