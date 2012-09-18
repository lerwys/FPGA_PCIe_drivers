#ifndef DMABUFFER_H_
#define DMABUFFER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2008-06-02 12:01:17  marcus
 * Added Sync commands for the DMA buffers. (testing)
 *
 * Revision 1.7  2008-03-20 13:26:56  marcus
 * Added support for DMA descriptors to be in user memory.
 *
 * Revision 1.6  2007-10-31 15:48:33  marcus
 * Added IG and KERNEL_PIECES tests.
 * Added Register access functions.
 *
 * Revision 1.5  2007-07-09 18:28:45  marcus
 * Improved UserMemory transfers.
 * Improved code organization.
 * Added support for concurrent user-memory DMA, but it is still in testing.
 *
 * Revision 1.4  2007-06-10 10:34:36  marcus
 * Backup commit. First tests with user memory buffers.
 *
 * Revision 1.3  2007/03/13 16:14:01  marcus
 * Added a Kernel Buffer to stored the descriptors for the SG list.
 *
 * Revision 1.2  2007/03/02 14:58:24  marcus
 * DMAEngineWG basic functionality working.
 *
 *******************************************************************/

#include <vector>
#include "DMADescriptor.h"

namespace pciDriver {
	class KernelMemory;
	class UserMemory;
}

namespace mprace {

class Board;
class DMADescriptorList;

/**
 * Interface for handling DMA buffers. 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.9 $
 * @date    $Date: 2009-05-04 13:37:52 $
 */
class DMABuffer {
	friend class DMAEngine;
	friend class DMAEngineWG;
	friend class DMADescriptorListWG;
public:
	/**
	 * The Type of DMA Buffer Memory used.
	 */
	enum MemType { 
		KERNEL, 		//>** Kernel Memory
		KERNEL_PIECES,	//>** Kernel with descriptors, used for debugging
		USER 			//>** User Memory
	};
	
	/**
	 * Describes the direction in which a buffer is synchronized
	 */
	enum SyncDir {
		BOTH = 0,		//>** Sync Memory in BOTH directions
		TODEVICE = 1,		//>** Sync Memory before a transfer TO the device
		FROMDEVICE = 2,		//>** Sync Memory after a transfer FROM the device 
	};
	
	/**
	 * Creates a DMA Buffer of the specified size and type.
	 * The buffer owns the memory, therefore, if will be
	 * deallocated when the buffer is destroyed.
	 * 
	 * @param board The board to associate this DMA Buffer with
	 * @param size Size of the buffer to allocate, in bytes
	 * @param type Type of the buffer (kernel/user memory).
	 * @param pieces Number of kernel pieces to divide the buffer. For debug only.
	 */
	DMABuffer(Board& board, const unsigned int size, MemType type, unsigned int kernel_pieces=1 );

	/**
	 * Creates a DMA Buffer of the specified size in user memory space.
	 * The buffer owns the memory, therefore, if will be
	 * deallocated when the buffer is destroyed.
	 * This call allows to select the alignment of the memory.
	 * 
	 * @param board The board to associate this DMA Buffer with
	 * @param size Size of the buffer to allocate, in bytes
	 * @param aligned If the memory is to be page-aligned or not.
	 */
	DMABuffer(Board& board, const unsigned int size, const bool aligned = true);
	
	/**
	 * Creates a DMA Buffer of the specified size, but
	 * does not owns the memory. Instead, it references a 
	 * provided pointer. Therefore, it is always user memory.
	 * 
	 * @param board The board to associate this DMA Buffer with
	 * @param size Size of the referenced memory area, in bytes
	 * @param ptr  Pointer to memory area of the buffer.
	 */
	DMABuffer(Board& board, const unsigned int size, unsigned int *ptr);

	
	/**
	 * Destroys a DMA Buffer.
	 * If the Buffer owns the memory, it also deallocates it.
	 */
	~DMABuffer();
	
	/**
	 * Synchronizes the contents of underlying buffers (if needed).
	 * @param dir Direction to sync
	 */
	void sync( SyncDir dir=BOTH ) const;
	
	/**
	 * Returns a user space memory pointer, to access the
	 * underlying buffer.
	 * @return A pointer to the buffer contents.
	 */
	inline unsigned int *getPointer() { return buf; }

	/**
	 * Return the DMA Buffer size.
	 */
	inline unsigned int size() const { return _size; }
	
	/**
	 * Access the underlying buffer as an array.
	 * Allows to use the buffer as an array of unsigned int elements.
	 * 
	 * @param index The index in the array.
	 */
	inline unsigned int& operator[](const unsigned int index)
		{ return buf[index]; }

	/**
	 * Get the current type of Memory of this buffer.
	 */
	inline MemType getType() const { return type; }

	/**
	 * Get the list of descriptors.
	 */
	inline DMADescriptorList& getDescriptors() { return *descriptors; }

	inline Board& getBoard() { return board; }
	
private:
	MemType type;			//**> The type of the memory used by the buffer
	unsigned int _size;		//**> Size of the memory, in bytes
	unsigned int *buf;		//**> Pointer in userspace to access the buffer area
	bool ownsMem;			//**> Whatever the buffer owns the memory or not.
	bool alignedMem;		//**> Whatever the User memory is page aligned or not (only if owned).
	unsigned int kernel_pieces;	//**> When using a MemType::KERNEL_PIECES, this variable keeps the number of pieces.

	Board& board;

	// These are implementation dependent. For PCIDriver only.
	pciDriver::KernelMemory *kBuf;	//**> Associated Kernel Buffer, if using a PCIDriver
	pciDriver::UserMemory *uBuf;	//**> Associated User Buffer, if using a PCIDriver

	DMADescriptorList *descriptors;

#if 0
	// old descriptor implementation. Replaced by DMADescriptorList class.
	// this is soon to be removed.
	pciDriver::KernelMemory *kBuf_descriptors;	//**> Associated Kernel Buffer to store the descriptors, if using a PCIDriver	
	void *descriptors;				//**> Pointer to the descriptors array.
#endif
	
}; /* class DMABuffer */

} /* namespace mprace */

#endif /*DMABUFFER_H_*/
