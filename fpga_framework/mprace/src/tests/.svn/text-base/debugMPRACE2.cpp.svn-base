
#include <string>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include "mprace/Board.h"
#include "mprace/MPRACE2.h"
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
#define FPGA_ADDR (0x100)
#define NLOOPS 50
//#define LOOPMAX 50000000
//#define LOOPMAX 500000
//#define LOOPMAX 500
//#define LOOPMAX 10
#define LOOPMAX 3

// Read, Write and Compare in one loop
//#define RWC
#define DMA_H2B
#define DMA_B2H

//#define B2H_ASYNC
//#define RESETLINK

#define MIN_SIZE 1024			// dwords
#define MAX_SIZE (1024*128*4)		// dwords --> 512KB
#define MAX_BLOCKRAM (4096) // dwords
#define MAX_SIZE_UMEM (1024*16384)		// dwords --> 64MB

//#define KERNELTEST
//#define USERTEST
//#define USERTEST_HUGEPAGES
#define PIO_CORRECT
//#define PIO_PERF
//#define DMA_CORRECT
//#define DMA_PERF
//#define DMA_CONCURRENT_PERF
//#define USE_INTERRUPTS
//#define INFLOOP


// Macros
#define asMB(x) (((double)(x))/(1024*1024))
#define asKB(x) ((double)(x)/1024)

// Global variable
unsigned int buf[MAX_SIZE];
unsigned int buf_read[MAX_SIZE];
bool verbose = false;

// Enums
enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

// Function prototypes
double testPIORperf(Board *board, unsigned int buf[], const unsigned int size, const bool block);
double testPIOWperf(Board *board, unsigned int buf[], const unsigned int size, const bool block);

double testDMARperf(Board *board, DMABuffer& buf, const unsigned int size);
double testDMAWperf(Board *board, DMABuffer& buf, const unsigned int size);
bool testDMAwrite(Board *board, DMABuffer& buf, const unsigned int size, const int pattern, const unsigned int mask=0xFFFFFFFF);
bool testDMAread(Board *board, DMABuffer& buf, const unsigned int size, const int pattern, const unsigned int mask=0xFFFFFFFF);

void doMemoryTest(Board *board, DMABuffer::MemType test);


int main(int argc, char **argv) {

#ifdef MULTITHREADED
	omp_set_dynamic(0);		// Disable dynamic adjustment
#endif

try {
	MPRACE2 *board = new MPRACE2(BOARD_NR);
	int i, temp;
	unsigned int val;
	unsigned int size;
	double t,mbps;
	int pattern;
	int mask = 0xFF00FF00;
	unsigned int loopCnt = 0, errCnt = 0, loopsOK = 0, loopsErr = 0;


#ifdef DMA_H2B
	DMABuffer buf_dma_h2b(*board, MAX_SIZE*4, DMABuffer::USER);
#endif
#ifdef DMA_B2H
	DMABuffer buf_dma_b2h(*board, MAX_SIZE*4, DMABuffer::USER);
#endif

	Timer::calibrate();

	if (argc < 3) {
		cerr << "Usage: testMPRACE2loop <pattern> <number-of-elements>" << endl
			 << "  <pattern> can be: random, random_ext, linear, linear_rev, circular, cycle2_circular" << endl << endl;
		return -1;
	}

	if (strcmp("random",argv[1])==0)
		pattern = RANDOM;
	else if (strcmp("random_ext",argv[1])==0)
		pattern = RANDOM_EXT;
	else if (strcmp("linear",argv[1])==0)
		pattern = LINEAR;
	else if (strcmp("linear_rev",argv[1])==0)
		pattern = LINEAR_REV;
	else if (strcmp("circular",argv[1])==0)
		pattern = CIRCULAR;
	else if (strcmp("cycle2_circular",argv[1])==0)
		pattern = CYCLE2_CIRCULAR;
	else {
		cerr << "Invalid pattern!!" << endl;
		cerr << "Usage: testMPRACE2loop <pattern> <number-of-elements>" << endl
			 << "  <pattern> can be: random, random_ext, linear, linear_rev, circular, cycle2_circular" << endl << endl;
		return -1;
	}

	size = atoi(argv[2]);

	// Print Design ID
	cout << "Design ID: " << hex << board->getBridgeReg(0) << dec << endl;

#ifdef MAIN_LOOPBACK
	cout << "MAIN LOOPBACK is active!" << endl;
#endif

	// reset the MGT links, clear the fifos
#ifdef RESETLINK
	cout << "Resetting MGT Links..." << flush;
	board->resetMGTlink();
	cout << "done." << endl;
#endif
	cout << "Resetting FIFOs..." << flush;
	board->setBridgeReg( MPRACE2::BRIDGE_TX_RESETFIFO, 0x0A );
	Timer::wait(0.1);
	board->setBridgeReg( MPRACE2::BRIDGE_LINK_REG, MPRACE2::BRIDGE_RESET_M2BFIFO );
	Timer::wait(0.1);
#ifndef MAIN_LOOPBACK
	board->setReg( MPRACE2::MAIN_RESET_FIFO, 0xFFFFFFFF );
	Timer::wait(0.1);
#endif
	cout << "done." << endl;

	cout << "Resetting DMA Channels..." << flush;
    board->getDMAEngine().reset(0);
    board->getDMAEngine().reset(1);
	cout << "done." << endl;

while (1) {
	if (loopCnt % 20 == 0)
		cout << "Loop " << loopCnt << endl;

	if (loopCnt == LOOPMAX) {
        cout << "Loop test finished, maximum reached." << endl;

        cout << endl
             << "Loops Total       : " << loopCnt << endl
             << "Loops OK          : " << loopsOK << endl
             << "Loops with Errors : " << loopsErr << endl;

        return 0;
    }

	// Fill the buffer
	srand( time(0) );
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

	// Clear read buffer
	for(i=0; i<size;i++)
		buf_read[i] = 0xaaaaffff;

	// Write buffer to FPGA
#if 0
	//for debug only
	// clear all fifos
	board->setReg( MPRACE2::MAIN_RESET_FIFO, 0xFFFFFFFF );
	board->setBridgeReg( MPRACE2::BRIDGE_LINK_REG, MPRACE2::BRIDGE_RESET_FIFO );
	board->setBridgeReg( MPRACE2::BRIDGE_LINK_REG, MPRACE2::BRIDGE_RESET_M2BFIFO );

	Timer::wait(0.1);
#endif

#ifdef RWC
	for(i=0; i<size;i++) {
		board->write(FPGA_ADDR+i, buf[i]);
		buf_read[i]=board->read(FPGA_ADDR+i);

		#ifdef MAIN_LOOPBACK
			if (buf[i] != ~(buf_read[i])) {
				cout << i << "> " << hex << buf[i] << " : " << ~(buf_read[i]) << dec << endl;
				errCnt++;
			}
		#else
			if (buf[i] != buf_read[i]) {
				cout << i << "> " << hex << buf[i] << " : " << buf_read[i] << dec << endl;
				errCnt++;
			}
		#endif

	}
#else
    // read
#ifdef B2H_ASYNC
 #ifdef DMA_B2H
	// Read data using DMA
	board->readDMA(FPGA_ADDR,buf_dma_b2h,size,0,true,false);
 #else
	board->readBlock(FPGA_ADDR, buf_read, size, true );
 #endif
#endif

#ifdef DMA_H2B
	// copy data to DMA buffer
	for(i=0; i<size;i++)
		buf_dma_h2b[i] = buf[i];

//	cout << "DMA buffer H2B cleared" << endl;

	// send data
	board->writeDMA(FPGA_ADDR,buf_dma_h2b,size,0,true,true);

//	cout << "DMA buffer H2B sent" << endl;
#else
	for(i=0; i<size;i++)
		board->write(FPGA_ADDR+i, buf[i]);
#endif

//	Timer::wait(0.005);

	// Read them back
#ifndef MAIN_LOOPBACK
		// clear both fifos
	board->setReg( MPRACE2::MAIN_RESET_FIFO, 0xFFFFFFFF );
	Timer::wait(0.0001);
#endif

		// read
#ifdef DMA_B2H
 #ifdef B2H_ASYNC
		// Wait for the previously initiated DMA read to finish
        volatile DMAEngine::DMAStatus rs;
        rs = board->getDMAEngine().getStatus(1);
        while (rs != DMAEngine::IDLE)
                rs = board->getDMAEngine().getStatus(1);
 #else
    	// Read data using DMA
    	board->readDMA(FPGA_ADDR,buf_dma_b2h,size,0,true,true);
 #endif

	// copy data from DMA buffer
	for(i=0; i<size;i++)
		buf_read[i] = buf_dma_b2h[i];

#else
 #ifndef B2H_ASYNC
	board->readBlock(FPGA_ADDR, buf_read, size, true );
 #endif
#endif

	// compare
	for(i=0;i<size;i++) {
	#ifdef MAIN_LOOPBACK
		if (buf[i] != ~(buf_read[i])) {
			cout << i << "> " << hex << buf[i] << " : " << ~(buf_read[i]) << dec << endl;
			errCnt++;
		}
	#else
		if (buf[i] != buf_read[i]) {
			cout << i << "> " << hex << buf[i] << " : " << buf_read[i] << dec << endl;
			errCnt++;
		}
	#endif
	}
#endif

	// report differences
	if (errCnt != 0)
		cout << "                   " << loopCnt << ": " << errCnt << endl;

    // add global counters
    if (errCnt != 0)
            loopsErr++;
    else
            loopsOK++;

	// lathe, rinse, repeat
	errCnt = 0;
	loopCnt++;
}

	return 0;

#ifdef PIO_CORRECT
#define ELEMS 16
	// clear main fifo
	board->setReg( MPRACE2::MAIN_RESET_FIFO, 0xFFFFFFFF );
	Timer::wait(1.0);

	for(i=0;i<ELEMS;i++) {
		buf[i] = -1;
		board->write(FPGA_ADDR+i,0xFFFF4320+i);
//		board->write(FPGA_ADDR+i,0xFF999900+i);
//		board->write(FPGA_ADDR+i,ELEMS-i);
	}
	Timer::wait(0.5);

	// clear both fifos
	board->setReg( MPRACE2::MAIN_RESET_FIFO, 0xFFFFFFFF );
	board->setBridgeReg( MPRACE2::BRIDGE_LINK_REG, MPRACE2::BRIDGE_RESET_FIFO );
	Timer::wait(1.0);

#if 1
	board->readBlock(FPGA_ADDR, buf, ELEMS, true );

	for(i=0;i<ELEMS;i++)
		cout << i << ": " << hex << buf[i] << dec << endl;
#else
	for(i=0;i<ELEMS;i++) {
		val = board->read(FPGA_ADDR+i);
		cout << i << ": " << val << endl;
	}
#endif

	// get error register
	val = board->getBridgeReg( MPRACE2::BRIDGE_ERROR_REG );
	cout << "REG_GEN_ERROR: " << hex << val << dec << endl;

#endif

	// Measure performance
#ifdef PIO_PERF
	cout << "Testing PIO Write performance: " << endl;
	for( size=MIN_SIZE; size<=MAX_SIZE; size*=2 ) {
		t = testPIOWperf(board, buf, size, false);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
		cout << " " << size*4 << " \t: " << mbps << " MBps" << "\t";
		t = testPIOWperf(board, buf, size, true);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
		cout << mbps << " MBps" << endl;
	}
	cout << endl;

	cout << "Testing PIO Read performance: " << endl;
	for( size=MIN_SIZE; size<=MAX_SIZE; size*=2 ) {
		t = testPIORperf(board, buf, size, false);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
		cout << " " << size*4 << " \t: " << mbps << " MBps" << "\t";
		t = testPIORperf(board, buf, size, true);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
		cout << mbps << " MBps" << endl;
	}
	cout << endl;
#endif


#ifdef KERNELTEST
	cout << "********************************************************************" << endl
		<<  "  KernelMemory DMA Tests" << endl
		<< "********************************************************************" << endl << endl;

	// Kernel Memory Test
	doMemoryTest(board,DMABuffer::KERNEL);

	cout << endl;
#endif

#ifdef USERTEST
	cout << "********************************************************************" << endl
		<<  "  UserMemory DMA Tests" << endl
		<< "********************************************************************" << endl << endl;

	// User Memory Test
	doMemoryTest(board,DMABuffer::USER);

	cout << endl;
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


void doMemoryTest(Board *board, DMABuffer::MemType type) {
	int i;
	unsigned int val;
	unsigned int size;
	double t,mbps;


	cout << "Testing DMA Buffers access from application: " << endl;
	for( size=MIN_SIZE; size<=MAX_BLOCKRAM; size*=2 ) {
		cout << asKB(size*sizeof(unsigned int)) << " kB\t: ";
		try {
			DMABuffer buf(*board, size*4, type);
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
		DMABuffer buf(*board, size*4, type);
		for(i=0;i<NLOOPS;i++)
			testDMAwrite(board, buf, size, pattern);
	}

	cout << endl << endl;

	cout << "Testing DMA Read: " << endl;
	for( size=MIN_SIZE; size<=MAX_BLOCKRAM; size*=2 )
	{
		DMABuffer buf(*board, size*4, type);
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
		DMABuffer buf(*board, size*4, type);
		t = testDMAWperf(board, buf, size);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
		cout << " " << asKB(size*4) << " kB\t: " << mbps << " MBps" << endl;
	}
	cout << endl;

	cout << "Testing DMA Read performance: " << endl;
	for( size=MIN_SIZE; size<=max_size; size*=2 ) {
		DMABuffer buf(*board, size*4, type);
		t = testDMARperf(board, buf, size);
		mbps = (t < 1e-12) ? 0.0 : asMB(size*4)/t;
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
		DMABuffer buf_in(*board, size*4, type);
		DMABuffer buf_out(*board, size*4, type);
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

