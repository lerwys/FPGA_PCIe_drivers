#include <string>
#include <iostream>
#include <exception>
#include <pthread.h>
#include <getopt.h>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>

#include <mprace/Board.h>
#include <mprace/DMABuffer.h>
#include <mprace/ABB.h>
#include <mprace/util/Timer.h>

using namespace std;
using namespace mprace;

#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define FIFO_ADDR 	(0x0)
#define TX_CTRL		(0x1E)
#define CLEAR_TIMEOUT	(0x0A)


int main(int argc, char *argv[]) {

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	Board *board;
	board = new ABB(BOARD_NR);

        board->setReg(TX_CTRL, CLEAR_TIMEOUT);
        cout << "Reset TX_CTRL" << endl;
}
