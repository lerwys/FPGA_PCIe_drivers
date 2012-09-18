#include "Board.h"
#include "Driver.h"
#include "DMADescriptor.h"
#include "DMADescriptorWG.h"
#include "DMADescriptorList.h"
#include "DMADescriptorListWG.h"
#include "DMABuffer.h"
#include "PCIDriver.h"
#include "pciDriver/lib/pciDriver.h"
#include <cstdlib>

using namespace mprace;
using namespace pciDriver;

#include <iostream>
using namespace std;


const unsigned int DMADescriptorListWG::page_size = 4096;
const unsigned int DMADescriptorListWG::desc_per_page = 4096 / sizeof(DMADescriptorWG::descriptor);


DMADescriptorListWG::DMADescriptorListWG( DMABuffer& parent, Type t )
	: DMADescriptorList(parent), type(t)
{
	// Get the number of descriptors, based on parent DMA buffer type.
	switch( parent.getType() ) {
	case DMABuffer::KERNEL_PIECES:
		this->size = parent.kernel_pieces;
		break;
	case DMABuffer::USER:
		this->size = parent.uBuf->getSGcount();
		break;
	default:
		throw "Unknown DMA Buffer Type!";
	}

	// Chreate the array of descriptor objects
	array = new DMADescriptorWG[ this->size ];

	// Initialize the native descriptor list
	this->init( this->size );

	// Link the descriptor objects to the native list
	this->link();
}

DMADescriptorListWG::~DMADescriptorListWG()
{
	delete[] blocks;
	delete[] array;

	if ((type == KERNEL) && (kBuf != NULL))
		delete kBuf;
	
	if ((type == USER) && (uBuf != NULL)) 
	{
		delete uBuf;
		free(native_ptr);
	}	
}

void DMADescriptorListWG::init( const unsigned int nr_descriptors ) 
{
	// Initialize native buffer
	PCIDriver& driver = dynamic_cast<PCIDriver&>( parent->getBoard().getDriver() );
	void *buf;
	unsigned int byte_size,nr_pages;
	
	switch(type) {
	case USER:
		nr_pages = nr_descriptors / desc_per_page;
		nr_blocks = ( nr_descriptors % desc_per_page == 0 ) ? nr_pages : nr_pages+1;
		byte_size = nr_blocks*page_size;

		posix_memalign(reinterpret_cast<void**>(&buf),page_size,byte_size);
		uBuf = &driver.mapUserMemory(buf,byte_size,false);

		// build the block list
		blocks = new block[nr_blocks];
		for(int i=0;i<nr_blocks;i++) {
			blocks[i].size = (i==(nr_blocks-1) && (nr_descriptors % desc_per_page)!=0) ? (nr_descriptors % desc_per_page) : desc_per_page;
			blocks[i].pa = uBuf->getSGentryAddress(i);
			blocks[i].ptr = static_cast<char*>(buf)+(i*page_size);
		}

#if 0
		for(int i=0;i<nr_blocks;i++) {
			cerr << i << ": " << hex
				<< (unsigned long)(blocks[i].ptr) << " " 
				<< (unsigned long long)(blocks[i].pa) << " " << dec
				<< (unsigned long long)(blocks[i].size) << endl; 
		}
#endif
		
		break;
	case KERNEL:
	default:
		nr_blocks = 1;
		byte_size = nr_descriptors * sizeof(DMADescriptorWG::descriptor);
		kBuf = &driver.allocKernelMemory( byte_size );
		buf = kBuf->getBuffer();

		// build the block list
		blocks = new block[1];
		blocks[0].size = nr_descriptors;
		blocks[0].ptr = kBuf->getBuffer();
		blocks[0].pa = kBuf->getPhysicalAddress();

#if 0
		cout << 0 << ": " << hex
			<< (unsigned long)(blocks[0].ptr) << " " 
			<< (unsigned long long)(blocks[0].pa) << " " << dec
			<< (unsigned long long)(blocks[0].size) << endl; 
#endif
		
		break;
	}
	
	native_ptr=buf;
}

void DMADescriptorListWG::link()
{
	unsigned int cur_block_index=0;
	unsigned int cur_block_count=0;

	unsigned long long pa;
	DMADescriptorWG::descriptor *native_block;

	block *cur_block = &(blocks[0]);
	native_block = static_cast<DMADescriptorWG::descriptor *>(cur_block->ptr);
	
	// Walk over the descriptors, linking them to their corresponding native descriptor
	for( int i=0 ; i<size ; i++,cur_block_count++) {
		DMADescriptorWG *cur_descriptor = &(array[i]);
		if (cur_block_count >= cur_block->size) {
			// go to next block
			cur_block_index++;
			cur_block = &(blocks[cur_block_index]);
			native_block = static_cast<DMADescriptorWG::descriptor *>(cur_block->ptr);
			cur_block_count=0;
		}
		
		// get the native descriptor addresses
		pa = cur_block->pa + (cur_block_count*sizeof(DMADescriptorWG::descriptor));
		cur_descriptor->setNativeDescriptor( native_block+cur_block_count );
		cur_descriptor->setPhysicalAddress(pa);		
	}
	
#if 0
	cout << "Linked Descriptors" << endl;
	for( int i=0 ; i<size ; i++) {
		DMADescriptorWG *cur_descriptor = &(array[i]);
		cout << i << ": ";
		cur_descriptor->printLine();
		cout << endl;
	}
#endif
	
}

void DMADescriptorListWG::print()
{
	cout << "DescriptorWG size: " << sizeof(DMADescriptorWG) << " bytes" << endl;
	cout << "Native DescriptorWG size: " << sizeof(DMADescriptorWG::descriptor) << " bytes" << endl;
	
	cout << "Blocks" << endl;
	for(int i=0 ; i < nr_blocks ; i++) {
		cout << i << ": ";		
		cout << hex 
			<< blocks[i].pa << " "
			<< blocks[i].size << " "
			<< (unsigned long long)blocks[i].ptr << dec << endl;
	}
	
	cout << "Linked Descriptors" << endl;
	for( int i=0 ; i<size ; i++) {
		DMADescriptorWG *cur_descriptor = &(array[i]);
		cout << i << ": ";
		cur_descriptor->printLine();
		cout << endl;
	}
	
}

void DMADescriptorListWG::sync()
{
	switch(type) {
	case KERNEL:
		kBuf->sync(KernelMemory::TO_DEVICE);
		break;
	case USER:
		uBuf->sync(UserMemory::TO_DEVICE);
		break;
	}
}
