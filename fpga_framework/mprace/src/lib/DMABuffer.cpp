/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2008-03-20 13:26:56  marcus
 * Added support for DMA descriptors to be in user memory.
 *
 * Revision 1.7  2007-10-31 15:49:47  marcus
 * Added IG and KERNEL_PIECES tests.
 * Added Register access functions.
 *
 * Revision 1.6  2007-07-09 18:28:48  marcus
 * Improved UserMemory transfers.
 * Improved code organization.
 * Added support for concurrent user-memory DMA, but it is still in testing.
 *
 * Revision 1.5  2007-07-06 19:20:36  marcus
 * New Register map for the ABB.
 * User Memory Support with Scatter/Gather Lists.
 *
 * Revision 1.4  2007-07-06 07:15:53  marcus
 * Minor change to support gcc 4.1
 *
 * Revision 1.3  2007-06-10 10:34:37  marcus
 * Backup commit. First tests with user memory buffers.
 *
 * Revision 1.2  2007/03/13 16:19:01  marcus
 * Added kBuf descriptor code, and calls to the fill- and release- descriptor methods.
 *
 * Revision 1.1  2007/03/02 14:58:24  marcus
 * DMAEngineWG basic functionality working.
 *
 *******************************************************************/

 
#include "Board.h"
#include "Driver.h"
#include "DMABuffer.h"
#include "DMAEngine.h"
#include "PCIDriver.h"
#include "Exception.h"
#include "pciDriver/lib/pciDriver.h"
#include <cstdlib>

using namespace mprace;
using namespace pciDriver;

DMABuffer::DMABuffer(Board& b, const unsigned int size, MemType t, unsigned int pieces )
	: board(b), type(t), ownsMem(true), _size(size), kernel_pieces(pieces)
{
	/* Needed only for DMABuffer::USER, but if not declared here,
	 * gcc 4.1.1 complaints. */
	unsigned int sz;
	unsigned int *ptr;
	
	descriptors = NULL;
	
	// Confirm we are using a PCIDriver, proceed accordingly
	if (PCIDriver *drv = dynamic_cast<PCIDriver*>( &b.getDriver() ) ) {

		// Proceed based on Memory type requested		
		switch (t) {
			case DMABuffer::KERNEL:
				kBuf = &drv->allocKernelMemory(size);
				uBuf = NULL;
				this->buf = static_cast<unsigned int *>(kBuf->getBuffer());
				break;
			case DMABuffer::KERNEL_PIECES:
				kBuf = &drv->allocKernelMemory(size);
				uBuf = NULL;
				this->buf = static_cast<unsigned int *>(kBuf->getBuffer());
				board.getDMAEngine().fillDescriptorList(*this);
				break;
			case DMABuffer::USER:
				sz = ((size % 4) == 0) ? (size >> 2) : ((size >> 2)+1);
#ifdef ALIGN_USEMEM
				posix_memalign(reinterpret_cast<void**>(&ptr),4096,sz*sizeof(unsigned int));
				this->alignedMem = true;
#else
				ptr = new unsigned int[sz];
				this->alignedMem = false;
#endif
				uBuf = &drv->mapUserMemory(ptr,size);
				kBuf = NULL;
				this->buf = ptr;

				// Fill the descriptors list of the buffer
				board.getDMAEngine().fillDescriptorList(*this);

				break;
			default:
				throw mprace::Exception( mprace::Exception::UNKNOWN );
		}		
	}
	else {
		// Throw exception
		throw mprace::Exception( mprace::Exception::UNKNOWN );
	}
}

DMABuffer::DMABuffer(Board& b, const unsigned int size, const bool aligned )
	: board(b), type(DMABuffer::USER), ownsMem(true), _size(size), kernel_pieces(0),
		kBuf(NULL)
{
	unsigned int sz;
	unsigned int *ptr;
	
	descriptors = NULL;
	
	sz = ((size % 4) == 0) ? (size >> 2) : ((size >> 2)+1);
	if (aligned) {
		posix_memalign(reinterpret_cast<void**>(&ptr),4096,sz*sizeof(unsigned int));
	} else {
		ptr = new unsigned int[sz];
	}
	this->buf = ptr;
	this->alignedMem = aligned;		

	// Confirm we are using a PCIDriver, proceed accordingly
	if (PCIDriver *drv = dynamic_cast<PCIDriver*>( &b.getDriver() ) )
		uBuf = &drv->mapUserMemory(ptr,size);

	// Fill the descriptors list of the buffer
	board.getDMAEngine().fillDescriptorList(*this);
}

DMABuffer::DMABuffer(Board& b, const unsigned int size, unsigned int *ptr)
	: board(b), type(DMABuffer::USER), ownsMem(false), _size(size)
{		
	// Confirm we are using a PCIDriver, proceed accordingly
	if (PCIDriver *drv = dynamic_cast<PCIDriver*>( &b.getDriver() ) ) {
		uBuf = &drv->mapUserMemory(ptr,size);
		this->buf = ptr;
		board.getDMAEngine().fillDescriptorList(*this);
	}
	else {
		// Throw exception
		throw mprace::Exception( mprace::Exception::UNKNOWN );
	}
}

DMABuffer::~DMABuffer()
{
	std::vector<DMADescriptor*>::const_iterator pos;

	// Delete kernel buffer
	if ((type == DMABuffer::KERNEL) && (kBuf != NULL))
		delete kBuf;

	// Delete kernel pieces buffer
	if ((type == DMABuffer::KERNEL_PIECES) && (kBuf != NULL))
		delete kBuf;
	
	// Delete User buffer
	if ((type == DMABuffer::USER) && (uBuf != NULL))  {
		delete uBuf;
		if (ownsMem) {
			if (this->alignedMem)	
				free(buf);
			else
				delete [] buf;
		}
	}
	
	// Free descriptors	
	board.getDMAEngine().releaseDescriptorList(*this);
}

void DMABuffer::sync(SyncDir dir) const
{	
	switch (type) {
	case DMABuffer::USER:
		UserMemory::sync_dir du;
		switch (dir) {
		case DMABuffer::FROMDEVICE:
			du = UserMemory::FROM_DEVICE;
			break;
		case DMABuffer::TODEVICE:
			du = UserMemory::TO_DEVICE;
			break;
		case DMABuffer::BOTH:
			du = UserMemory::BIDIRECTIONAL;
			break;
		default:
			// ERROR
			break;
		}

		uBuf->sync(du);
		
		break;
		
	case DMABuffer::KERNEL:
		KernelMemory::sync_dir dk;
		switch (dir) {
		case DMABuffer::FROMDEVICE:
			dk = KernelMemory::FROM_DEVICE;
			break;
		case DMABuffer::TODEVICE:
			dk = KernelMemory::TO_DEVICE;
			break;
		case DMABuffer::BOTH:
			dk = KernelMemory::BIDIRECTIONAL;
			break;
		default:
			// ERROR
			break;
		}

		kBuf->sync(dk);
		
		break;
		
	default:
		// ERROR
		break;
	}
}
