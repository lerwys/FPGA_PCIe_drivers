/*******************************************************************
 * This is a test program for ABB Board runnning a sample design.
 * 
 * $Revision: 1.3 $
 * $Date: 2007-03-01 16:59:55 $
 * 
 *******************************************************************/

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/12/11 16:16:13  marcus
 * Backup commit.
 *
 * Revision 1.1  2006/11/21 15:54:21  marcus
 * Moved board tests to a separate directory, 'other_tests'.
 *
 * Revision 1.3  2006/11/13 12:30:54  marcus
 * Added an interrupt test.
 *
 * Revision 1.2  2006/10/31 07:57:56  marcus
 * Added test with User Memory.
 *
 * Revision 1.1  2006/10/30 19:39:01  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "lib/pciDriver.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include <pthread.h>

using namespace pciDriver;
using namespace std;

#define MAX 16
#define KBUF_SIZE (4096)
#define UBUF_SIZE (4096)

void testDevice( int i );
void testBARs(pciDriver::PciDevice *dev);
void testDirectIO(pciDriver::PciDevice *dev);
void testDMA(pciDriver::PciDevice *dev);
void testInterrupts(pciDriver::PciDevice *dev);

class BDA {
public:
	unsigned long src_addr_h;
	unsigned long src_addr_l;
	unsigned long dst_addr_h;
	unsigned long dst_addr_l;
	unsigned long length;
	unsigned long control;
	unsigned long next_bda_h;
	unsigned long next_bda_l;
	unsigned long status;
	
	void write(unsigned long *base) {
		base[0] = src_addr_h;
		base[1] = src_addr_l;
		base[2] = dst_addr_h;
		base[3] = dst_addr_l;
		base[4] = next_bda_h;
		base[5] = next_bda_l;
		base[6] = length;
		base[7] = control;		// control is written at the end, starts DMA
	}
	
	void reset(unsigned long *base) {
		base[7] = 0x0000000A;
	}
};


class DDList {
public:
	DDList(int size) {
		lod = new BDA[size];
		this->count = size;
	}
	~DDList() {
		delete lod;
	}
	
	int length() { return count; }
	
	BDA& operator[](int index) {
		return lod[index];
	}
private:
	int count;
	BDA *lod;
};

int main() 
{
	int i;
	
	for(i=0;i<4;i++) {
		testDevice( i );
	}

	return 0;
}

void testDevice( int i ) {
	pciDriver::PciDevice *device;
		
	try {
		cout << "Trying device " << i << " ... ";
		device = new pciDriver::PciDevice( i );
		cout << "found" << endl;

		testBARs(device);
		testDirectIO(device);
		testDMA(device);
//		testInterrupts(device);

		delete device;

	} catch (Exception& e) {
		cout << "failed: " << e.toString() << endl;
		return;
	}

}

void testBARs(pciDriver::PciDevice *dev)
{
	int i;
	void *bar;
	unsigned int size;
	
	dev->open();
	
	for(i=0;i<6;i++) {
		cout << "Mapping BAR " << i << " ... ";
		try {
			bar = dev->mapBAR(i);
			cout << "mapped ";
			dev->unmapBAR(i,bar);
			cout << "unmapped ";
			size = dev->getBARsize(i);
			cout << size << endl;
		} catch (Exception& e) {
			cout << "failed" << endl;
		}
	}

	dev->close();
}

void testDirectIO(pciDriver::PciDevice *dev)
{
	unsigned int *bar0, *bar1;
	unsigned long long *bar2;
	unsigned int bar0size, bar1size, bar2size;
	unsigned long long *longbar0;
	unsigned int i, val;
	
	unsigned int buf[UBUF_SIZE];

	try {
		// Open device
		dev->open();

		// Map BARs
	    bar0 = static_cast<unsigned int *>( dev->mapBAR(0) );
	    bar1 = static_cast<unsigned int *>( dev->mapBAR(1) );
//	    bar2 = dev->mapBAR(2);

		longbar0 = (unsigned long long*)bar0;

		// Get BAR sizes
		bar0size = dev->getBARsize(0);
		bar1size = dev->getBARsize(1);
//		bar2size = dev->getBARsize(2);

		// test register memory
		bar0[0] = 0x1234565;
		bar0[1] = 0x5aa5c66c;

		for(i=0;i<MAX;i++) {
			val = bar0[i];
			cout << hex << setw(8) << val << endl;
		}		

		// write block
		for(i=0;i<MAX;i++) {
			buf[i] = ~i;
		}
		memcpy( (void*)bar0, (void*)buf, MAX*sizeof(unsigned int) );

		// read block
		memcpy( (void*)buf, (void*)bar0, MAX*sizeof(unsigned int) );
		for(i=0;i<MAX;i++) {
			val = buf[i];
			cout << hex << setw(8) << val << endl;
		}
		cout << endl << endl;

		// Unmap BARs
		dev->unmapBAR(0,bar0);
		dev->unmapBAR(1,bar1);
//		dev->unmapBAR(2,bar2);

		// Close device
		dev->close();
		
	} catch(Exception& e) {
		cout << "Exception: " << e.toString() << endl;
	}	
}

void writeDMA( unsigned long *bar1, unsigned long pa, unsigned long addr, unsigned long next, unsigned long size, bool inc, bool block )
{
	BDA dma;
	const unsigned long BASE_DMA_DOWN = (0x0C040 >> 2);

	// TODO: add inc to control word
	// TODO: add lastDescriptor to control word, based on 'next'

	dma.reset(bar1+BASE_DMA_DOWN);

	// Send a DMA transfer
	dma.src_addr_h = 0x00000000;
	dma.src_addr_l = pa;
	dma.dst_addr_h = 0x00000000;
	dma.dst_addr_l = addr;
	dma.length = size;
	dma.control = 0x01008000;		
	dma.next_bda_h = 0x00000000;
	dma.next_bda_l = next;

	dma.write(bar1+BASE_DMA_DOWN);

	if (block) {
		// TODO: wait for the finished status
		sleep(5);
	}	
}


void readDMA( unsigned long *bar1, unsigned long pa, unsigned long addr, unsigned long next, unsigned long size, bool inc, bool block )
{
	BDA dma;
	const unsigned long BASE_DMA_UP   = (0x0C000 >> 2);

	// TODO: add inc to control word
	// TODO: add lastDescriptor to control word, based on 'next'

	dma.reset(bar1+BASE_DMA_UP);

	// Send a DMA transfer
	dma.src_addr_h = 0x00000000;
	dma.src_addr_l = addr;
	dma.dst_addr_h = 0x00000000;
	dma.dst_addr_l = pa;
	dma.length = size;
	dma.control = 0x01008000;		
	dma.next_bda_h = 0x00000000;
	dma.next_bda_l = next;

	dma.write(bar1+BASE_DMA_UP);	

	if (block) {
		// TODO: wait for the finished status
		sleep(5);	
	}	
}


void testDMAKernelMemory( 
		unsigned long *bar0, 
		unsigned long *bar1, 
		KernelMemory *km, 
		const unsigned long BRAM_SIZE )
{
	BDA dma;
	int err,i;
	
	unsigned long *base;
	unsigned long *ptr;

	ptr = static_cast<unsigned long *>( km->getBuffer() );

	//**** DMA Write (down)

	// fill buffer
	// send buffer
	// compare buffer
	
	//**** DMA Read (up)
	
	// fill buffer with simple IO
	// read buffer
	// compare buffer
	

	cout << "Fill buffer with zeros" << endl;
	memset( ptr, 0, BRAM_SIZE );

	writeDMA( bar1, km->getPhysicalAddress(), 0x00000000, 0x00000000, BRAM_SIZE, true, true );

	// Check
	cout << "Checking BRAM..." << flush;
	for(err=0,i=0;i<(BRAM_SIZE >> 2);i++) 
		if ( bar0[i] != 0 ) err++;
	if (err==0)
		cout << "err" << endl;

	// Print contents of the BlockRAM
	for(i=0;i<(BRAM_SIZE >> 2);i++) 
		cout << setw(4) << hex << i*4 << ": " << setw(8) << bar0[i] << endl;
	cout << endl;

	// second write
	cout << "Fill buffer with a pattern" << endl;
	// fill with pattern
	for(i=0;i<(BRAM_SIZE >> 2);i++) {
		if ((i & 0x00000001) == 0)
			ptr[i] = 0x5555aaaa;
		else
			ptr[i] = 0xaaaa5555;
	}

	writeDMA( bar1, km->getPhysicalAddress(), 0x00000000, 0x00000000, BRAM_SIZE, true, true );

	// Print contents of the BlockRAM
	for(i=0;i<(BRAM_SIZE >> 2);i++) 
		cout << setw(4) << hex << i*4 << ": " << setw(8) << bar0[i] << endl;
	cout << endl;

	//**** DMA Read (up)
	// From Device to Host. Uses DMA UP

	// Clear buffer
	cout << "Fill buffer with zeros:" << endl;
	memset( ptr, 0x0, BRAM_SIZE );

	readDMA( bar1, km->getPhysicalAddress(), 0x00000000, 0x00000000, BRAM_SIZE, true, true );

	// Get Buffer contents
	cout << "Get Buffer content after DMA" << endl;
	for(i=0;i<(BRAM_SIZE >> 2);i++) 
		cout << setw(4) << hex << i*4 << ": " << setw(8) << ptr[i] << endl;
	cout << endl << endl;

	// Clear buffer
	cout << "Clear FPGA area with zeros:" << endl;
	memset( bar0, 0, BRAM_SIZE );

	readDMA( bar1, km->getPhysicalAddress(), 0x00000000, 0x00000000, BRAM_SIZE, true, true );

	// Get Buffer contents
	cout << "Get Buffer content after DMA" << endl;
	for(i=0;i<(BRAM_SIZE >> 2);i++) 
		cout << setw(4) << hex << i*4 << ": " << setw(8) << ptr[i] << endl;
	cout << endl << endl;
}


void testDMAUserMemory( 
		unsigned long *bar0, 
		unsigned long *bar1, 
		KernelMemory *km, 
		const unsigned long BRAM_SIZE,
		const unsigned long BASE_DMA_UP,
		const unsigned long BASE_DMA_DOWN )
{
	BDA dma;	
	unsigned long *base;

#if 0
//************************************************************
// write something else to the FPGA area before the next test

		for(i=0;i<MAX;i++) {
			bar0[i] = ~i;
		}
		cout << "Filled FPGA area with test data" << endl << endl;

//************************************************************
		// Now with User Memory

		cout << "Using User Memory: " << endl << endl;

		cout << "Scatther / Gather List: " << endl;
		for(i=0;i<um->getSGcount();i++) {
			cout << i << ": " << hex << setw(8) 
				<< um->getSGentryAddress(i) << " - "
				<< um->getSGentrySize(i) << endl;
		}

		if ((um->getSGcount() > 1) && (um->getSGentrySize(0) < MAX)) {
			cerr << "**** Scatthered, and Size is less than the tranferred words" << endl;
			
			delete km;
			delete um;
			dev->close();
			return;
		}
		
		//**** DMA Write
		
		// Clear buffer
		cout << "Fill buffer with zeros" << endl;
		memset( buf, 0, MAX*sizeof(unsigned int) );
		for(i=0;i<MAX;i++) {
			cout << hex << setw(8) << buf[i] << endl;
		}
		cout << endl << endl;
		
		// Send 8 words in a DMA transfer
		dmaSize = 8;
		dmaCmd = DMA_WRITE_CMD + dmaSize;
		bar1[0] = dmaCmd;						// DMA command
		bar1[1] = um->getSGentryAddress(0);		// Host Physical Address
		bar1[2] = 0;							// FPGA destination Address

		// wait
		sleep(5);

		um->sync(UserMemory::BIDIRECTIONAL);
		
		// Print contents of the buffer
		cout << "After DMA: " << endl;
		for(i=0;i<MAX;i++) {
			cout << hex << setw(8) << buf[i] << endl;
		}
		cout << endl << endl;

		//**** DMA Read

		// Clear buffer
		cout << "Fill buffer with 0x5a:" << endl;
		memset( buf, 0x5a, MAX*sizeof(unsigned int) );
		for(i=0;i<MAX;i++) {
			cout << hex << setw(8) << buf[i] << endl;
		}
		cout << endl << endl;

		// Clear FPGA area
		cout << "Clear FPGA area with zeros" << endl;
		for(i=0;i<MAX;i++) {
			bar0[i] = 0;
		}
		for(i=0;i<MAX;i++) {
			cout << hex << setw(8) << bar0[i] << endl;
		}
		cout << endl << endl;

		um->sync(UserMemory::BIDIRECTIONAL);

		// Send 8 words in a DMA transfer
		dmaSize = 8;
		dmaCmd = DMA_READ_CMD + dmaSize;
		bar1[0] = dmaCmd;						// DMA command
		bar1[1] = um->getSGentryAddress(0);		// Host Physical Address
		bar1[2] = 0;							// FPGA destination Address

		// wait
		sleep(5);

		// Get FPGA area contents
		cout << "Get FPGA area after DMA" << endl;
		for(i=0;i<MAX;i++) {
			cout << hex << setw(8) << bar0[i] << endl;
		}
		cout << endl << endl;
#endif	

}


void testDMA(pciDriver::PciDevice *dev)
{
	KernelMemory *km;
	UserMemory *um;
	unsigned int i, val;
	unsigned long *ptr;
	unsigned long *bar0, *bar1;

	const unsigned long BRAM_SIZE = 0x4000;
	unsigned int umBuf[BRAM_SIZE];


	unsigned long *base;

	try {
		// Open device
		dev->open();

		// Map BARs
	    bar0 = static_cast<unsigned long *>( dev->mapBAR(0) );
	    bar1 = static_cast<unsigned long *>( dev->mapBAR(1) );

		// Create buffers
		km = &dev->allocKernelMemory( BRAM_SIZE );
		um = &dev->mapUserMemory( umBuf ,BRAM_SIZE, true );

		// Test Kernel Buffer

		cout << "Kernel Buffer User address: " << hex << setw(8) << ptr << endl;
		cout << "Kernel Buffer Physical address: " << hex << setw(8) << km->getPhysicalAddress() << endl;
		cout << "Kernel Buffer Size: " << hex << setw(8) << km->getSize() << endl;

		testDMAKernelMemory( bar0, bar1, km, BRAM_SIZE );
//		testDMAUserMemory( bar0, bar1, um, BRAM_SIZE );

		// Delete buffer descriptors
		delete km;
		delete um;

		// Unmap BARs
		dev->unmapBAR(0,bar0);
		dev->unmapBAR(1,bar1);
		
		// Close device
		dev->close();

	} catch(Exception&e) {
		cout << "Exception: " << e.toString() << endl;
	}	
}


/* Global used by the interrupt thread */
pthread_mutex_t int_mutex;
bool thread_alive;
unsigned long int_count;

void *int_handler(void *t) {
	pciDriver::PciDevice *dev = static_cast<pciDriver::PciDevice*>(t);
	
	while(thread_alive) {
		dev->waitForInterrupt();
		int_count++;
	}

	// signal the other thread we are exiting
	pthread_mutex_unlock( &int_mutex );
	
}


void testInterrupts(pciDriver::PciDevice *dev)
{
	KernelMemory *km;
	unsigned int i, val;
	unsigned int buf[UBUF_SIZE];
	unsigned int *ptr;
	unsigned int *bar0, *bar1;

	const unsigned int DMA_WRITE_CMD = 0xc0000000;
	const unsigned int DMA_READ_CMD  = 0x80000000;
	const unsigned int IRQ_ON_CMD    = 0x40000000;
	const unsigned int IRQ_OFF_CMD   = 0x00000000;

	unsigned int dmaCmd;
	unsigned int dmaSize;

	// Create a thread to handle the interrupts
	pthread_t tid;

	int_count  = 0;
	thread_alive = true;
	pthread_create( &tid, NULL, int_handler, dev );
	pthread_mutex_init( &int_mutex, NULL );
	
	try {
		// Open device
		dev->open();

		// Map BARs
	    bar0 = static_cast<unsigned int *>( dev->mapBAR(0) );
	    bar1 = static_cast<unsigned int *>( dev->mapBAR(1) );

		// The interrupt test is simply a turn on/ turn off switch.

		// Create buffers
		km = &dev->allocKernelMemory( KBUF_SIZE );

		cout << "Enabling interrupts" << endl;

		// Turn interrupt On
		dmaCmd = IRQ_ON_CMD + dmaSize;
		bar1[0] = dmaCmd;						// DMA command
		bar1[1] = km->getPhysicalAddress();		// Host Physical Address
		bar1[2] = 0;							// FPGA destination Address

		cout << "Waiting interrupts...";
			
		// Wait 5 Secs
		sleep(2);

		cout << "done" << endl;
		
		// Turn interrupt Off
		dmaCmd = IRQ_OFF_CMD + dmaSize;
		bar1[0] = dmaCmd;						// DMA command
		bar1[1] = km->getPhysicalAddress();		// Host Physical Address
		bar1[2] = 0;							// FPGA destination Address
		
		// Output number of interrupts received
		cout << "Interupts received: " << int_count << endl;

		// Clear threads
		thread_alive = false;
		pthread_mutex_lock( &int_mutex );	// will unlock on exit of the other thread		
		pthread_mutex_destroy( &int_mutex );
				
		// Delete buffer descriptors
		delete km;

		// Unmap BARs
		dev->unmapBAR(0,bar0);
		dev->unmapBAR(1,bar1);
		
		// Close device
		dev->close();

	} catch(Exception& e) {
		cout << "Exception: " << e.toString() << endl;
	}
	
	pthread_mutex_destroy( &int_mutex );	
}
