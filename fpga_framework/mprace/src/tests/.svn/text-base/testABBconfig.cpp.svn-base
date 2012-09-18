
#include <string>
#include <iostream>
#include <exception>
#include <cstdlib>
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

// Enums
enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

// Defines
#define BOARD_NR 1
#define MIN_SIZE 1024			// dwords
#define MAX_SIZE (1024*128)		// dwords
#define MAX_BLOCKRAM (4096) // dwords
#define FPGA_ADDR 0x0
#define NLOOPS 50

#define DMA_CORRECT
//#define PIO_PERF
#define DMA_PERF

// Macros
#define asMB(x) (((double)(x))/(1024*1024))
#define asKB(x) ((double)(x)/1024)

bool verbose = false;


bool testDMAwrite(Board *board, DMABuffer& buf, const unsigned int size, const int pattern, const unsigned int mask=0xFFFFFFFF);

int main(int argc, char **argv) {

#ifdef MULTITHREADED
	omp_set_dynamic(0);		// Disable dynamic adjustment
#endif

	if (argc < 2) {
		cout << "Usage: " << argv[0] << " config-filename " << endl;
		return -1;
	}

try {
	Board *board = new ABB(BOARD_NR);
	int i;
	unsigned int val;
	unsigned int size=1024;

	Timer::calibrate();

	DMABuffer buf(*board, size*4, DMABuffer::KERNEL);
	DMABuffer buf2(*board, size*4, DMABuffer::USER);

	unsigned int pattern = RANDOM;

	testDMAwrite(board, buf, size, pattern);
	testDMAwrite(board, buf2, size, pattern);
	
//	board->config(argv[1]);
	
} catch (exception& e) {
	cout << e.what() << endl;
}	
	return 0;
}

bool testDMAwrite(Board *board, DMABuffer& buf, const unsigned int size, int pattern, const unsigned int mask ) {
	unsigned int i,errcnt;
	bool err=false;
	unsigned int temp;

	// Prefill the BLOCKRAM with a known value as reference
	if (verbose)
		cout << "filling " << flush;

	for(i=0;i<MAX_BLOCKRAM;i++)
		board->write(i,i);
	
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
	board->writeDMA(0,buf,size,0,true,true);
	
	// Compare
	if (verbose)
		cout << "comparing " << flush;
	errcnt=0;
	for(i=0;i<size;i++)
		if (buf[i] != board->read(i))  { 
			if (verbose)
				cout << hex << buf[i] << " : " << board->read(i) << endl;
			errcnt++;
		}
	if (verbose)
		cout << "done. " << errcnt << endl;
	
	return !err;
}
