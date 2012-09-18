#ifndef DMADESCRIPTORLISTWG_H_
#define DMADESCRIPTORLISTWG_H_

#include <mprace/DMADescriptorWG.h>
#include <mprace/DMADescriptorList.h>

namespace pciDriver {
	class KernelMemory;
	class UserMemory;
}

namespace mprace {

class DMABuffer;
class DMADescriptorWG;

class DMADescriptorListWG : public DMADescriptorList {
public:
	/**
	 * Possible types for the native descriptor list.
	 */
	enum Type {
		KERNEL,		//**> User Kernel Memory for the native descriptors
		USER		//**> Use User Memory for the native descriptors
	};
	
	DMADescriptorListWG( DMABuffer& parent, Type t = KERNEL );
	
	~DMADescriptorListWG();

	inline DMADescriptorWG& operator[](const unsigned int idx)
		{ return array[idx]; } 

	inline unsigned int getSize()
		{ return size; }
	
	void print();

	void sync();
	
protected:

private:
	const static unsigned int page_size;		//**> page size
	const static unsigned int desc_per_page;	//**> descriptors per page
	
	Type type;								//**> Type of Buffer used for the native descriptors
	
	unsigned int size;						//**> Number of entries (descriptors)
	DMADescriptorWG *array;					//**> Array of Descriptor objects
	
	typedef struct {
		unsigned int size;					//**> in native descriptors
		void *ptr;							//**> starting address of the block (in user space)
		unsigned long long pa;				//**> starting physical address of the block
	} block;
	
	void *native_ptr;				//**> Pointer to the native buffer space (in user memory)
	unsigned int nr_blocks;			//**> Number of blocks
	block *blocks;					//**> Blocks used for the native descriptor list
	
	// Native Buffer areas
	// These are implementation dependent. For PCIDriver only.
	pciDriver::KernelMemory *kBuf;	//**> Associated Kernel Buffer, if using a PCIDriver
	pciDriver::UserMemory *uBuf;	//**> Associated User Buffer, if using a PCIDriver
	
	void init(const unsigned int nr_descriptors);
	
	void link();
	
};	// end class

}	// end namespace


#endif /*DMADESCRIPTORLISTWG_H_*/
