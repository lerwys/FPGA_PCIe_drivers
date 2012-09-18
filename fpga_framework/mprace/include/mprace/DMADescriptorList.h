#ifndef DMADESCRIPTORLIST_H_
#define DMADESCRIPTORLIST_H_

#include "DMABuffer.h"

namespace mprace {

/**
 * Abstract class to the Descriptor List. Most subclasses are instantiated from a template,
 * but they are DMAEngine dependent, so they can be of any form, and have any interface. 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2008-03-20 13:26:56 $
 */
class DMADescriptorList {
public:
	virtual ~DMADescriptorList() {}
	
	virtual void print()=0;
protected:
	
	/**
	 * Creates a DMA Descriptor List. Protected because only subclasses should 
	 * be instantiated.
	 */
	DMADescriptorList() {}
	
	DMADescriptorList(DMABuffer& _parent) : parent(&_parent) {}

	DMABuffer *parent;		//**> DMA Buffer who owns this descriptor list.
	
private:
	
};	// end class

}	// end namespace

#endif /*DMADESCRIPTORLIST_H_*/
