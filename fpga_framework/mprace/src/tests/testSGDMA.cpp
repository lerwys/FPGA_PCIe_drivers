
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

using namespace mprace;
using namespace mprace::util;
using namespace std;

// Defines
#define BOARD_NR 1
#define KBUF_SIZE (4*1024*1024)
#define PIECES_MIN 1
#define PIECES_MAX 4096
#define NLOOPS 50

bool verbose = false;

#ifdef OLD_REGISTERS
 #define FPGA_ADDR (0x0)
#else
 #define FPGA_ADDR (0x8000 >> 2)
#endif


// Macros
#define asMB(x) (((double)(x))/(1024*1024))
#define asKB(x) ((double)(x)/1024)

double testDMARperf(Board *board, DMABuffer& buf, const unsigned int size);
double testDMAWperf(Board *board, DMABuffer& buf, const unsigned int size);

int main(int argc, char **argv) {

try {
	Board *board = new ABB(BOARD_NR);
	int i;
	unsigned int val;
	unsigned int pieces;
	unsigned int size = (KBUF_SIZE/4);
	double time,mbps;

	Timer::calibrate();

	cout << "Design ID: " << hex << board->getReg(0) << dec << endl;

	
	cout << "Kernel Buffer Size: " << KBUF_SIZE << endl;
	
	cout << "Testing DMA performance: " << endl;
	for( pieces = PIECES_MIN; pieces <= PIECES_MAX; pieces *= 2 ) {
		DMABuffer buf(*board, size*4, DMABuffer::KERNEL_PIECES, pieces);
		
		// write
		time = testDMAWperf(board, buf, size);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << " " << pieces << "\t " << asKB(size*4) << " kB\t" << mbps << " MBps";

		// read
		time = testDMARperf(board, buf, size);
		mbps = (time < 1e-12) ? 0.0 : asMB(size*4)/time;
		cout << "\t" << mbps << " MBps" << endl;
	}
	cout << endl;

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
