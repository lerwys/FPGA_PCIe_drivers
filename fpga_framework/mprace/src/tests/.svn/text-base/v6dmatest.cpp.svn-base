#include <string>
#include <iostream>
#include <exception>
#include <pthread.h>
#include <getopt.h>
#include <cstdlib>
#include <cstdio>

#include <mprace/Board.h>
#include <mprace/DMABuffer.h>
#include <mprace/ABB.h>
#include <mprace/util/Timer.h>

using namespace std;
using namespace mprace;
using namespace mprace::util;


#define BOARD_NR 	0

#define NLOOPS          8
#define MAX_BLOCKS      4096

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define FIFO_ADDR 	(0x0)
#define TX_CTRL		(0x1E)
#define CLEAR_TIMEOUT	(0x0A)


/*                        */
/*  --------------------  */
/*                        */

int main(int argc, char *argv[]) {

	Timer t;
	Board *board;
	board = new ABB(BOARD_NR);

	int i, j;
	int size = 1024;

	DMABuffer buf(*board, (1*MAX_BLOCKS*size * sizeof(int)), DMABuffer::USER);

	cout << endl << "Testing DMA write performance ..." << endl;
#if 1
	for (j=256; j<=MAX_BLOCKS; j*=2) {
		for (i = 0; i < size; i++)
			buf[i] = i;

		t.start();
		for (i=0; i<NLOOPS; i++){
//			board->writeDMA(FPGA_ADDR, buf, j*size, 0, true, true);
//			cout << "Writing buffer with size " << j * size << " (" << i << " / " << NLOOPS << ")"<<  endl;
			board->writeDMA(FPGA_ADDR, buf, j*size, 0, false, true);
		}
		t.stop();
		double wperf = j*size*4.0*NLOOPS/1024.0/1024.0/t.asSeconds();
		cout << (j*4) << " KB : " <<  wperf << " MBps" << endl;
	}

#endif

#if 1
	cout << endl << "Testing DMA read performance ..." << endl;
	for (j=256; j<=MAX_BLOCKS; j*=2) {
		t.start();
		for (i=0; i<NLOOPS; i++){
//			board->readDMA(FPGA_ADDR, buf, j*size, 0, true, true);
			board->readDMA(FPGA_ADDR, buf, j*size, 0, false, true);
		}
		t.stop();
		double rperf = j*size*4.0*NLOOPS/1024.0/1024.0/t.asSeconds();
		cout << (j*4) << " KB : " <<  rperf << " MBps" << endl;
	}

	cout << endl;
#endif

	return 0;
}
