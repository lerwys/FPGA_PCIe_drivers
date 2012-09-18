
#include <string>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstring>

#include "mprace/Board.h"
#include "mprace/ABB.h"
#include "mprace/util/Timer.h"
#include "mprace/DMABuffer.h"
#include "mprace/Exception.h"

// This requires the use of the Intel C Compiler
#ifdef MULTITHREADED
#include <omp.h>
#endif

using namespace mprace;
using namespace mprace::util;
using namespace std;

// Defines
#define BOARD_NR 0
#define MIN_SIZE 1024			// dwords
#define MAX_SIZE (1024*128*8)		// dwords --> 512KB
#define MAX_SIZE_UMEM (1024*16384)		// dwords --> 64MB
#define MAX_BLOCKRAM (4096) // dwords

#ifdef OLD_REGISTERS
 #define FPGA_ADDR (0x0)
#else
 #define FPGA_ADDR (0x8000 >> 2)
#endif

#define NLOOPS 50
#define KERNELPIECES 100

#define KERNELTEST
//#define KERNELPIECESTEST
#define USERTEST
//#define USERTEST_HUGEPAGES
//#define PIO_CORRECT
//#define PIO_PERF
#define DMA_CORRECT
#define DMA_PERF
//#define DMA_CONCURRENT_PERF
#define USE_INTERRUPTS
#define INFLOOP

// Macros
#define asMB(x) (((double)(x))/(1024*1024))
#define asKB(x) ((double)(x)/1024)
// Global variable
unsigned int buf[MAX_SIZE];
bool verbose = false;

// Enums
enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

// Function prototypes
double testPIORperf(Board *board, unsigned int buf[], const unsigned int size, const bool block);
double testPIOWperf(Board *board, unsigned int buf[], const unsigned int size, const bool block);

double testDMARperf(Board *board, DMABuffer& buf, const unsigned int size);
double testDMAWperf(Board *board, DMABuffer& buf, const unsigned int size);
void testDMARWperf(Board *board, DMABuffer& buf_in, DMABuffer& buf_out, const unsigned int size, double *rtime, double *wtime);

bool testDMAwrite(Board *board, DMABuffer& buf, const unsigned int size, const int pattern, const unsigned int mask=0xFFFFFFFF);
bool testDMAread(Board *board, DMABuffer& buf, const unsigned int size, const int pattern, const unsigned int mask=0xFFFFFFFF);

void doMemoryTest(Board *board, DMABuffer::MemType type, DMABuffer& buf, DMABuffer& buf_in, DMABuffer& buf_out );

int main(int argc, char **argv) {

#ifdef MULTITHREADED
	omp_set_dynamic(0);		// Disable dynamic adjustment
#endif

try {
	Board *board = new ABB(BOARD_NR);
	DMAEngineWG& dma = static_cast<DMAEngineWG&>(board->getDMAEngine());
	int i;
	unsigned int val;
	unsigned int size;
	double time,mbps;

	Timer::calibrate();

	cout << "Design ID: " << hex << board->getReg(0) << dec << endl;

	cout << "Initializing DMA Buffers..." << flush;
	
#ifdef KERNELTEST	
	DMABuffer buf_k(*board, MAX_SIZE*4, DMABuffer::KERNEL );
	DMABuffer buf_kin(*board, MAX_SIZE*4, DMABuffer::KERNEL  );
	DMABuffer buf_kout(*board, MAX_SIZE*4, DMABuffer::KERNEL );
#endif

#ifdef KERNELPIECESTEST	
	DMABuffer buf_kp(*board, MAX_SIZE*4, DMABuffer::KERNEL_PIECES, KERNELPIECES );
	DMABuffer buf_kpin(*board, MAX_SIZE*4, DMABuffer::KERNEL_PIECES, KERNELPIECES  );
	DMABuffer buf_kpout(*board, MAX_SIZE*4, DMABuffer::KERNEL_PIECES, KERNELPIECES );
#endif

#ifdef USERTEST	
	DMABuffer buf_u(*board, MAX_SIZE_UMEM*4, DMABuffer::USER );
	DMABuffer buf_uin(*board, MAX_SIZE_UMEM*4, DMABuffer::USER  );
	DMABuffer buf_uout(*board, MAX_SIZE_UMEM*4, DMABuffer::USER );
#endif
	
#ifdef INFLOOP
	int loopCount=0;
	while(1) {
		
		cout << "Loop Count: " << loopCount << endl << endl;
#endif

	cout << endl << " Disabling interrupts ..." << flush;
	dma.setUseInterrupts(false);
	cout << "done." << endl << endl;
		
#ifdef PIO_CORRECT
	for(i=0;i<16;i++)
		board->write(FPGA_ADDR+i,16-i);
		
	for(i=0;i<16;i++) {
		val = board->read(FPGA_ADDR+i);
		cout << i << ": " << val << endl;
	}
#endif

	// Prepare buffer
	srand( ::time(NULL) );
	for(i=0;i<MAX_SIZE;i++)
		buf[i] = rand();

	// Measure performance
#ifdef PIO_PERF
	cout << "Testing PIO Write performance: " << endl;
	for( size=MIN_SIZE; size<=MAX_SIZE; size*=2 ) {
		time = testPIOWperf(board, buf, size, false);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << " " << size*4 << " \t: " << mbps << " MBps" << "\t";
		time = testPIOWperf(board, buf, size, true);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << mbps << " MBps" << endl;
	}
	cout << endl;

	cout << "Testing PIO Read performance: " << endl;
	for( size=MIN_SIZE; size<=MAX_SIZE; size*=2 ) {
		time = testPIORperf(board, buf, size, false);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << " " << size*4 << " \t: " << mbps << " MBps" << "\t";
		time = testPIORperf(board, buf, size, true);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << mbps << " MBps" << endl;
	}
	cout << endl;
#endif

#ifdef KERNELTEST
	cout << "********************************************************************" << endl
		<<  "  KernelMemory DMA Tests" << endl
		<< "********************************************************************" << endl << endl;

	// Kernel Memory Test
	doMemoryTest(board, DMABuffer::KERNEL, buf_k, buf_kin, buf_kout);
	
	cout << endl;
#endif

#ifdef KERNELPIECESTEST
	cout << "********************************************************************" << endl
		<<  "  KernelPieces Memory DMA Tests" << endl
		<< "********************************************************************" << endl << endl;

	// Kernel Memory Test
	doMemoryTest(board, DMABuffer::KERNEL_PIECES, buf_kp, buf_kpin, buf_kpout);
	
	cout << endl;
#endif
	
#ifdef USERTEST
	cout << "********************************************************************" << endl
		<<  "  UserMemory DMA Tests" << endl
		<< "********************************************************************" << endl << endl;
	
	// User Memory Test
	doMemoryTest(board, DMABuffer::USER, buf_u, buf_uin, buf_uout);

	cout << endl;
#endif		

#ifdef USERTEST_HUGEPAGES
	cout << "********************************************************************" << endl
		<<  "  UserMemory DMA Tests (HugePages)" << endl
		<< "********************************************************************" << endl << endl;
	
	// User Memory Test
	doMemoryTest(board,DMABuffer::USER_HUGEPAGES);

	cout << endl;
#endif		


#ifdef USE_INTERRUPTS
	cout << endl << " Enabling interrupts ..." << flush;
	dma.setUseInterrupts(true);
	cout << "done." << endl << endl;

#ifdef KERNELTEST
	cout << "********************************************************************" << endl
		<<  "  KernelMemory DMA Tests (w/interrupts)" << endl
		<< "********************************************************************" << endl << endl;

	// Kernel Memory Test
	doMemoryTest(board, DMABuffer::KERNEL, buf_k, buf_kin, buf_kout);
	
	cout << endl;
#endif

#ifdef KERNELPIECESTEST
	cout << "********************************************************************" << endl
		<<  "  KernelPieces Memory DMA Tests (w/interrupts)" << endl
		<< "********************************************************************" << endl << endl;

	// Kernel Memory Test
	doMemoryTest(board, DMABuffer::KERNEL_PIECES, buf_kp, buf_kpin, buf_kpout);
	
	cout << endl;
#endif
	
#ifdef USERTEST
	cout << "********************************************************************" << endl
		<<  "  UserMemory DMA Tests (w/interrupts)" << endl
		<< "********************************************************************" << endl << endl;
	
	// User Memory Test
	doMemoryTest(board, DMABuffer::USER, buf_u, buf_uin, buf_uout);

	cout << endl;
#endif		

#ifdef USERTEST_HUGEPAGES
	cout << "********************************************************************" << endl
		<<  "  UserMemory DMA Tests (HugePages, w/interrupts)" << endl
		<< "********************************************************************" << endl << endl;
	
	// User Memory Test
	doMemoryTest(board,DMABuffer::USER_HUGEPAGES);

	cout << endl;
#endif		
	

#endif

#ifdef INFLOOP
		++loopCount;
	
	}
#endif


} catch (mprace::Exception& e) {
	if (e.getParameter() != 0)
		cout << e.what() << " (" << strerror(e.getParameter()) << ")" << endl;
	else
		cout << e.what() << endl;
} catch (exception& e) {
	cout << e.what() << endl;
}	
	return 0;
}

void doMemoryTest(Board *board, DMABuffer::MemType type, DMABuffer& buf, DMABuffer& buf_in, DMABuffer& buf_out) {
	int i;
	unsigned int val;
	unsigned int size;
	double time,mbps;

	cout << "Testing DMA Buffers access from application: " << endl;
	for( size=MIN_SIZE; size<=MAX_BLOCKRAM; size*=2 ) {
		cout << asKB(size*sizeof(unsigned int)) << " kB\t: ";
		try {
			for(int i=0;i<size;i++)
				buf[i] = size-i;
				
			for(int i=0;i<size;i++)
				if (buf[i] != (size-i)) {
					cout << "failed buffer comp" << endl;
					break;
				}
			cout << "ok" << endl;
		} catch (mprace::Exception& e) {
			cout << "fail: " << e.what() << endl;
		}	
	}

#ifdef DMA_CORRECT
	// Pattern to use for the tests
	unsigned int pattern = RANDOM;
	
	cout << "Testing DMA Write: " << endl;
	for( size=MIN_SIZE; size<=MAX_BLOCKRAM; size*=2 )
	{
		for(i=0;i<NLOOPS;i++)
			testDMAwrite(board, buf, size, pattern);
	}	

	cout << endl << endl;
	
	cout << "Testing DMA Read: " << endl;
	for( size=MIN_SIZE; size<=MAX_BLOCKRAM; size*=2 )
	{
		for(i=0;i<NLOOPS;i++)
			testDMAread(board, buf, size, pattern);
	}	

	cout << endl << endl;
#endif

	unsigned int max_size;
	if (type ==DMABuffer::USER)
		max_size = MAX_SIZE_UMEM;
	else
		max_size = MAX_SIZE;
		
	
#ifdef DMA_PERF
	cout << "Testing DMA Write performance: " << endl;
	for( size=MIN_SIZE; size<=max_size; size*=2 ) {
		time = testDMAWperf(board, buf, size);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << " " << asKB(size*4) << " kB\t: " << mbps << " MBps" << endl;
	}
	cout << endl;

	cout << "Testing DMA Read performance: " << endl;
	for( size=MIN_SIZE; size<=max_size; size*=2 ) {
		time = testDMARperf(board, buf, size);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << " " << asKB(size*4) << " kB\t: " << mbps << " MBps" << endl;
	}
	cout << endl;
#endif

#ifdef DMA_CONCURRENT_PERF
	cout << "Testing Concurrent DMA Read and Write performance: " << endl;
	cout << "size \t: read - write" << endl;
	for( size=MIN_SIZE; size<=max_size; size*=2 ) {
		double rtime, wtime;
		double rmbps, wmbps;
		testDMARWperf(board, buf_in, buf_out, size, &rtime, &wtime);
		rmbps = (rtime < 1e-12) ? 0.0 : asMB(size*4)/rtime;
		wmbps = (wtime < 1e-12) ? 0.0 : asMB(size*4)/wtime;
		cout << " " << asKB(size*4) << " kB\t: " << rmbps << " MBps" << " - " << wmbps << " MBps" << endl;
	}
	cout << endl;
#endif	
		
}

double testPIORperf(Board *board, unsigned int buf[], const unsigned int size, const bool block) {
	unsigned int i,j;
	Timer t;

	
	if (block) {
		t.start();
		for(i=0;i<NLOOPS;i++)
			board->readBlock(FPGA_ADDR, buf, size, false);
		t.stop();	
	}
	else {
		t.start();
		for(j=0;j<NLOOPS;j++) {
			for(i=0;i<size;i++) {
				buf[i] = board->read(FPGA_ADDR);
			}
		}
		t.stop();
	}

	return t.asSeconds()/NLOOPS;
}

double testPIOWperf(Board *board, unsigned int buf[], const unsigned int size, bool block) {
	unsigned int i,j;
	Timer t;
	
	if (block) {
		t.start();
		for(i=0;i<NLOOPS;i++)
			board->writeBlock(FPGA_ADDR, buf, size, false);
		t.stop();	
	}
	else {
		t.start();
		for(j=0;j<NLOOPS;j++) {
			for(i=0;i<size;i++) {
				board->write(FPGA_ADDR,buf[i]);
			}
		}
		t.stop();
	}

	return t.asSeconds()/NLOOPS;
}

bool testDMAwrite(Board *board, DMABuffer& buf, const unsigned int size, int pattern, const unsigned int mask ) {
	unsigned int i,errcnt;
	bool err=false;
	unsigned int temp;

	// Prefill the BLOCKRAM with a known value as reference
	if (verbose)
		cout << "filling " << flush;

	for(i=0;i<MAX_BLOCKRAM;i++)
		board->write(FPGA_ADDR+i,i);
	
	// Fill the buffer
	switch (pattern) {
		case LINEAR:
			for(i=0;i<size;i++)
				buf[i] = i;
			break;
		case LINEAR_REV:
			for(i=0;i<size;i++)
				buf[i] = size-i;
			break;
		case CIRCULAR:
			for(i=0;i<size;i++) {
				temp = ((temp % 32) == 0) ? 0x00000001 : (temp << 1);
				buf[i]=temp;
			}
			break;
		case CYCLE2_CIRCULAR:
			for(i=0;i<size;i++) {
				temp = ((temp % 16) == 0) ? 0x00010001 : (temp << 1);
				buf[i]=temp;
			}
			break;
		case RANDOM:
			for(i=0;i<size;i++)
				buf[i] = rand();
			break;
		case RANDOM_EXT:
			for(i=0;i<size;i++)
				buf[i] = rand() & mask;
			break;
			
	}

	
	// Send
	if (verbose)
		cout << "sending " << flush;
	board->writeDMA(FPGA_ADDR,buf,size,0,true,true);
	
	// Compare
	if (verbose)
		cout << "comparing " << flush;
	errcnt=0;
	for(i=0;i<size;i++)
		if (buf[i] != board->read(FPGA_ADDR+i))  { 
			if (verbose)
				cout << hex << buf[i] << " : " << board->read(FPGA_ADDR+i) << endl;
			errcnt++;
		}
	if (verbose)
		cout << "done. " << errcnt << " of " << size << endl;
	else {
		if (errcnt == 0)
			cout << ".";
		else
			cout << "x";
	}
	
	return !err;
}

bool testDMAread(Board *board, DMABuffer& buf, const unsigned int size, int pattern, const unsigned int mask) {
	unsigned int i,errcnt,temp;
	bool err=false;

	// Fill the buffer on the FPGA
	switch (pattern) {
		case LINEAR:
			for(i=0;i<size;i++)
				board->write(FPGA_ADDR+i,i);
			break;
		case LINEAR_REV:
			for(i=0;i<size;i++)
				board->write(FPGA_ADDR+i,size-i);
			break;
		case CIRCULAR:
			for(i=0;i<size;i++) {
				temp = ((temp % 32) == 0) ? 0x00000001 : (temp << 1);
				board->write(FPGA_ADDR+i,temp);
			}
			break;
		case CYCLE2_CIRCULAR:
			for(i=0;i<size;i++) {
				temp = ((temp % 16) == 0) ? 0x00010001 : (temp << 1);
				board->write(FPGA_ADDR+i,temp);
			}
			break;
		case RANDOM:
			for(i=0;i<size;i++)
				board->write(FPGA_ADDR+i,rand());
			break;
		case RANDOM_EXT:
			for(i=0;i<size;i++)
				board->write(FPGA_ADDR+i,rand() & mask);
			break;
			
	}

	// Clear the DMA buffer prior to transfer
	for(i=0;i<size;i++)
		buf[i] = i;
		
	// Read the DMA buffer
	if (verbose)
		cout << "reading " << flush;
	board->readDMA(FPGA_ADDR,buf,size,0,true,true);
	
	// Compare
	if (verbose)
		cout << "comparing " << flush;
	errcnt=0;
	for(i=0;i<size;i++)
		if (buf[i] != board->read(FPGA_ADDR+i))  { 
			if (verbose)
				cout << hex << buf[i] << " : " << board->read(FPGA_ADDR+i) << endl;
			errcnt++;
		}
	if (verbose)
		cout << "done. " << errcnt << " of " << size << endl;
	else {
		if (errcnt == 0)
			cout << ".";
		else
			cout << "x";
	}
	
	return !err;
	
}

double testDMARperf(Board *board, DMABuffer& buf, const unsigned int size) {
	unsigned int i,j;
	Timer t;
	
	t.start();
	for(i=0;i<NLOOPS;i++)
		board->readDMA(FPGA_ADDR, buf, size, 0, false, true);
	t.stop();	

	return t.asSeconds()/NLOOPS;
}

double testDMAWperf(Board *board, DMABuffer& buf, const unsigned int size) {
	unsigned int i,j;
	Timer t;
	
	t.start();
	for(i=0;i<NLOOPS;i++)
		board->writeDMA(FPGA_ADDR, buf, size, 0, false, true);
	t.stop();	

	return t.asSeconds()/NLOOPS;
}

void testDMARWperf(Board *board, DMABuffer& buf_in, DMABuffer& buf_out, const unsigned int size, double *rtime, double *wtime)
{
	unsigned int ri,wi;
	Timer tr, tw;
	volatile unsigned int ws, rs;
	
	ri = NLOOPS;
	wi = NLOOPS;

	ws = board->getDMAEngine().getStatus(0);
	rs = board->getDMAEngine().getStatus(1);

	*rtime = 0.0;
	*wtime = 0.0;


#ifdef MULTITHREADED
	int iam,np;
	
#pragma omp parallel num_threads(2) dafault(shared) private(iam,np)
{
	iam = omp_get_thread_num();
	np = omp_get_num_threads();

	// Do the read
	if (iam == 0) {
		tr.start();
		for(ri=0;ri<NLOOPS;ri++)
			board->readDMA(FPGA_ADDR, buf_in, size, 0, false, true);
		tr.stop();
		
		rtime = tr.asSeconds();
	}

	// Do the write	
	if (iam == 1) {
		tw.start();
		for(wi=0;wi<NLOOPS;wi++)
			board->writeDMA(FPGA_ADDR, buf_out, size, 0, false, true);
		tw.stop();	

		wtime = tw.asSeconds();
	}		
}

#else
	while ((ri > 0) && (wi > 0)) {

		// start transfers if not busy
		if ((ri > 0) && (rs == DMAEngine::IDLE)) {
			tr.start();
			board->readDMA(FPGA_ADDR, buf_in, size, 0, false, false);
		}
		
		if ((wi > 0) && (ws == DMAEngine::IDLE)) {
			tw.start();
			board->writeDMA(FPGA_ADDR, buf_out, size, 0, false, false);		
		}

		// check status
		ws = board->getDMAEngine().getStatus(0);
		rs = board->getDMAEngine().getStatus(1);

		// if finished, process timings.
		if (rs == DMAEngine::IDLE) {
			tr.stop();
			*rtime += tr.asSeconds();
			--ri;
		}
		
		if (ws == DMAEngine::IDLE) {
			tw.stop();
			*wtime += tw.asSeconds();
			--wi;
		}
		
	}
#endif

	// average over the loops
	*rtime = (*rtime) / NLOOPS;
	*wtime = (*wtime) / NLOOPS;
}

