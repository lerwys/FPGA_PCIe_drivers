/**
 * Test-case which uses the data generator on the ABB to generate 8 MB of
 * data and do a DMA read performance test afterwards.
 *
 * @file testDGen.cpp
 * @author Michael Stapelberg
 * @date 2009-07-20
 *
 */
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

#include "DataGenerator.hpp"

using namespace std;
using namespace mprace;


#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define NLOOPS		1
#define DG_OFFSET       (0xC0000 >> 2)
#define DG_CTRL    	(0x00A8  >> 2)
#define DG_RESET	(0x00A)

#define FIFO_ADDR 	(0x0)
#define TX_CTRL		(0x1E)
#define CLEAR_TIMEOUT	(0x0A)

/** Converts the given double to megabytes */
#define TO_MiB(bytes) ((bytes) / (1024 * 1024))

/** Output verbose messages like which bytes failed in each test */
static bool verbose = false;


/** Global instance of the board */
Board *board;

static void test_dma_read_performance(Board *board, DMABuffer &buf,
				      unsigned int offset, unsigned int size)
{
	unsigned int i;
	mprace::util::Timer timer;

	cout << "Starting FIFO DMA read performance test, reading " << size << " dwords" << endl;

	timer.start();
	for (i = 0; i < NLOOPS; i++) {
		/* Read, but don't increment the offset on the FPGA, so
		   we don't go out of bounds (we read more often than
		   we reserved buffer space) for */
		board->readDMAFIFO(FIFO_ADDR + offset, buf, size, 0, false, true);
	}
	timer.stop();

	/* averaged time for a DMA read */
	double time = (timer.asSeconds() / NLOOPS);
	/* prevent division by zero */
	double performance;

	if (time < 1e-12)
		performance = 0;
	else performance = TO_MiB((size * sizeof(int)) / time);

	cout << "DMA read performance: " << performance << " MiB" << endl;
}

/**
 *
 * @return Returns success (0) if the test was passed without errors and
 * failure (1) if otherwise.
 *
 */
int main(int argc, char *argv[]) {
	/* seed random() */
	srand(time(NULL));

	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);

	cout << "Clear Tx Time-out" << endl;
	board->setReg(TX_CTRL, CLEAR_TIMEOUT);

	mprace::ABBDataGenerator gen(board);


	// Clear the FIFO
	cout << "Clear FIFO" << endl;
	board->setReg(0x24, 0x0A);
	/* As long as the status register is 0, the FIFO is still clearing */
	while (board->getReg(0x24) == 0) {
		/* wait until the clearing of the FIFO is finished */
	}
	cout << "Status reg = " << board->getReg(0x24) << endl;

	cout << "Data Generator is " <<((board->getReg(0x08)&0x0020)?"":"NOT ") << "present." << endl;
	cout << "HW version  " << hex << board->getReg(0x0) << endl;
	cout << "FIFO status : " << board->getReg(0x24) << endl;

//      // Enable DGen, set it to IDLE state
//      cout << "Set DGen to IDLE" << endl;
//      board->setReg(0x00A8, 0x0100);

	cout << "Data Generator Status : " << board->getReg(0x2A) << endl;
	cout << "Filling the DataGenerator table..." << endl;

	int sz = 2048; /* size of the pattern in bytes */

	uint16_t pattern[32] = {0xFF0E, 0x5062, 0x7083, 0x90A4,
				0xB0C5, 0xD0E6, 0xF007, 0x1020,

				0x00AA, 0x00BB, 0x00CC, 0x00DD,
				0xAA00, 0xBB00, 0xCC00, 0xDD00,

				0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD,
				0x9999, 0x8888, 0x7777, 0x6666,
				0x5555, 0x4444, 0x3333, 0x2222,
				0x1111, 0x0000, 0x1100, 0x2200};

	uint16_t *bigpattern = (uint16_t*)malloc(sz * 2 * sizeof(uint16_t));
	if (bigpattern == NULL) {
		cerr << "malloc failed" << endl;
		return 1;
	}
	/* Fill up the big pattern buffer repeatedly with our 16 byte pattern */
	for (int i = 0; i < sz * 2; i++) {
		int w = i % 32;
		bigpattern[i] = pattern[w];
	}

	cout << "Letting the data generator loop until nearly full..." << endl;

	gen.storePattern(true, 2 * sz, bigpattern);

	/* Check for the nearly full bit repeatedly */
	while ((board->getReg(0x24)  & 0x2) == 0) {
		cout << "Waiting until its full" << endl;
	}
	cout << "FIFO status : " << hex << board->getReg(0x24) << endl;
	cout << "Data Generator Status : " << hex << board->getReg(0x2A) << endl;

	sz *= 1024;
	DMABuffer buf(*board, (4 * sz * sizeof(int)), DMABuffer::USER);

	gen.stop();
	test_dma_read_performance(board, buf, 0, sz);

	cout << "Stopped the dgen, fifo status = " << board->getReg(0x24) << endl;
	int fifo_status = board->getReg(0x24);
	fifo_status &= 0xFFFFFFF8;
	cout << "packets in the fifo = " << fifo_status << endl;

	test_dma_read_performance(board, buf, 0, sz);

	cout << "FIFO status : " << board->getReg(0x24) << endl;

	return 0;
}
