#include <iostream>

#include <mprace/Board.h>
#include <mprace/DMABuffer.h>
#include <mprace/ABB.h>

#define BOARD_NR 0
#define FIFO_ADDR 	(0x0)

using namespace std;
using namespace mprace;

int main()
{
        const int size = 1024;
        Board *board;

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);
	board->setReg(0x04, 0x0010);

        DMABuffer *buf;

        buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);
        (*buf)[0] = 0;
        (*buf)[1] = 0;

        cout << "Reading DMA FIFO" << endl;
	int c=50;
        //while (c--) {
        board->readDMAFIFO(FIFO_ADDR, *buf, size, 0, true, true, 1);
        //}

        cout << "Done." << endl;
        cout << "buf[0] = " << (*buf)[0] << ", buf[1] = " << (*buf)[1] << endl;
}
