#ifndef DMADESCRIPTORWG_H_
#define DMADESCRIPTORWG_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2008-03-20 13:26:56  marcus
 * Added support for DMA descriptors to be in user memory.
 *
 *******************************************************************/
 
#include <iostream>
#include <mprace/DMADescriptor.h>

#define HIGH(x) (static_cast<unsigned int>((static_cast<unsigned long long>((x))) >> 32))
#define LOW(x) (static_cast<unsigned int>((static_cast<unsigned long long>((x))) & (0x00000000FFFFFFFF)))

namespace mprace {

class DMADescriptor;


/**
 * Describes a DMA Descriptor as used by the DMAEngineWG. 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.7 $
 * @date    $Date: 2009-06-10 22:17:16 $
 */
class DMADescriptorWG : public DMADescriptor {
	friend class DMAEngineWG;
public:
	/**
	 * Internal structure representing the actual descriptor the engine expects
	 * to see in memory. Native descriptor lists are made of this type.
	 */
	struct descriptor_s {
		unsigned int per_addr_h;
		unsigned int per_addr_l;
		unsigned int host_addr_h;
		unsigned int host_addr_l;
		unsigned int next_bda_h;
		unsigned int next_bda_l;
		unsigned int length;
		unsigned int control;
				
	} __attribute__ ((aligned (64)));
	/*
	 *  Request the compiler to align the descriptor to a 64-byte boundary.
	 *  This is required because: 
	     (a) Descriptors list must not cross a page boundary (4k)
	     (b) It is best to comply to the RCB in PCIe, not to split the descriptor
	         transfer into multiple completions.
	 */
	
	typedef struct descriptor_s descriptor;
	
	DMADescriptorWG() : owned(true)
	{ 
		this->native = new descriptor;
	}

	DMADescriptorWG( DMADescriptorWG::descriptor *nativeDescriptor ) : owned(false)
	{
		this->native = nativeDescriptor;
	}
	
	DMADescriptorWG( DMADescriptorWG::descriptor *nativeDescriptor, const unsigned long long pa, const unsigned long long per, const unsigned long long host, const unsigned long l, const unsigned long long next, unsigned long ctrl)
		: owned(false)
	{
		this->native = nativeDescriptor;
		setPhysicalAddress( pa );
		setPeripheralAddress( per );
		setHostAddress( host );
		setNextDescriptorAddress( next );
		setLength( l );
		setControl( ctrl );
	}

	virtual ~DMADescriptorWG() 
	{ 
		if ((this->native != NULL) && (this->owned))
			delete this->native;		
	}

	inline unsigned long long getPhysicalAddress() 
		{ return pa; }

	inline unsigned long long getHostAddress()
		{ return ((static_cast<unsigned long long>(native->host_addr_h) << 32) + native->host_addr_l); }

	inline unsigned long long getPeripheralAddress()
		{ return ((static_cast<unsigned long long>(native->per_addr_h) << 32) + native->per_addr_l); }

	inline unsigned long long getNextDescriptorAddress()
		{ return ((static_cast<unsigned long long>(native->next_bda_h) << 32) + native->next_bda_l); }

	inline unsigned long getLength()
		{ return native->length; } 

	inline unsigned long getControl()
		{ return native->control; } 

	inline void setNativeDescriptor( DMADescriptorWG::descriptor *nativeDescriptor )
	{
		if ((this->native != NULL) && (this->owned))
			delete this->native;
		
		this->native = nativeDescriptor; owned = false; 
	}

	inline DMADescriptorWG::descriptor *getNativeDescriptor()
		{ return this->native; }
	
	inline void setPhysicalAddress(const unsigned long long addr) 
		{ pa = addr; }

	inline void setPeripheralAddress(const unsigned long long addr)
		{ native->per_addr_h = HIGH(addr); native->per_addr_l = LOW(addr); }
		
	inline void setHostAddress(const unsigned long long addr)
		{ native->host_addr_h = HIGH(addr); native->host_addr_l = LOW(addr); }

	inline void setNextDescriptorAddress(const unsigned long long addr)
		{ native->next_bda_h = HIGH(addr); native->next_bda_l = LOW(addr); }

	inline void setLength( const unsigned long l )
		{ native->length = l; }
	
	inline void setControl( const unsigned long ctrl )
		{ native->control = ctrl; }

	DMADescriptorWG& operator=(DMADescriptorWG & src) {
		if (native == NULL)
			return *this;
		
		native->per_addr_h = src.native->per_addr_h;
		native->per_addr_l = src.native->per_addr_l;
		native->host_addr_h = src.native->host_addr_h;
		native->host_addr_l = src.native->host_addr_l;
		native->next_bda_h = src.native->next_bda_h;
		native->next_bda_l = src.native->next_bda_l;
		native->length = src.native->length;
		native->control = src.native->control;

		return *this;
	}

	inline void printLine() {
		std::cout << std::hex
			<< "[" << getPhysicalAddress() << "] "
			<< getHostAddress() << " "
			<< getPeripheralAddress() << " "
			<< getNextDescriptorAddress() << " "
			<< getLength() << " "
			<< getControl() << std::dec;
	}
	
protected:

private:
	DMADescriptorWG::descriptor *native;	//>** Pointer to the native descriptor.
	bool owned;								//>** We own the native descriptor (therefore we are responsible to delete it).
	
	// This is not required in the native descriptor, but it is used to build the
	// the list.
	unsigned long long pa;
	
}; /* class DMADescriptorWG */

} /* namespace mprace */


#endif /*DMADESCRIPTORWG_H_*/
