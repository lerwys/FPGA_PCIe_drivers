#ifndef DRIVER_H_
#define DRIVER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007-05-29 07:50:44  marcus
 * Backup commit.
 *
 * Revision 1.1  2007/02/12 18:09:16  marcus
 * Initial commit.
 *
 *******************************************************************/

namespace mprace {

/**
 * Abstract interface to any driver supported by the library.
 *  Allows the separation of the low level device driver from the
 * library, by defining the operations needed by the library to
 * be performed by the driver. Each driver instance represents one
 * device.
 * 
 *  As a difference with the uelib, non-PCI drivers are supported.
 * To accomplish this, the concept of an 'area' is defined. For PCI
 * devices, an area is a BAR. For USB devices, an area represents 
 * an endpoint buffer. Numbering of the areas is device specific.
 * 
 *  Another difference is that the DMA Engine is completely abstracted
 * from the driver. If the DMA operation is hidden or not, depends
 * on the implementation of the DMAEngine and the DMABuffer used.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.3 $
 * @date    $Date: 2008-01-11 10:30:21 $
 */
class Driver {
public:
	/**
	 * Delete a device.
	 * Must be virtual to properly call the driver in the subclass.
	 */
	virtual ~Driver() { }

	/**
	 * Open the device.
	 */
	virtual void open() =0;
	/**
	 * Close the device.
	 */
	virtual void close() =0;

	/**
	 * Memory Map an area into User Space Memory.
	 * @param num the area number to mmap.
	 * @return A pointer to access the requested area.
	 * @exception mprace::Exception on Error.
	 */
	virtual void *mmapArea(const unsigned int num) =0;
	
	/**
	 * Release an area previously mapped into User Space Memory.
	 * @param num the area number to release.
	 * @exception mprace::Exception on Error.
	 */
	virtual void unmapArea(const unsigned int num) =0;
	
	/**
	 * Get the size of the area.
	 * @param num the area number.
	 * @return the size of the area, in bytes.
	 */
	virtual unsigned int getAreaSize(const unsigned int num) =0;

	/**
	 * Wait for an Interrupt to arrive.
	 * @param int_id The IRQ id number to identify the waiting queue in the driver.
	 * This is a blocking call.
	 */
	virtual void waitForInterrupt(unsigned int int_id)=0;
	
protected:
	Driver() { }
	
}; /* class Driver */

} /* namespace mprace */

#endif /*DRIVER_H_*/
