
/**
 * This is the test case for the ABB board with parallel access
 *
 * @file testParallelABB.cpp
 * @author Michael Stapelberg
 * @date 2009-03-30
 *
 */
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

pthread_mutex_t dma_read_mutex;
pthread_mutex_t dma_write_mutex;

enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

/** Number of times the DMA read or write will be done to get decent performance
   measurement values */
#define NLOOPS 		2
// #define NLOOPS 		50

/** Minimum size of the buffer used for DMA */
static int MIN_SIZE = 1024;  //2048;
//static int MIN_SIZE = 1024;

/** Maximum size of the buffer used for DMA */
static int MAX_BLOCKRAM = 2048;  //4096;


#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define FIFO_ADDR 	(0x0)
#define TX_CTRL		(0x1E)
#define CLEAR_TIMEOUT	(0x0A)

/** Converts the given double to megabytes */
#define TO_MiB(bytes) ((bytes) / (1024 * 1024))

/** Output verbose messages like which bytes failed in each test */
static bool verbose = false;

/** This option is only useful for debugging, it will disable DMA entirely. */
static bool use_pio_only = false;

static bool use_fifo = false;

/** The pattern to use for tests */
static int pattern = LINEAR_REV;

/** Global instance of the board */

/**
 * Returns the next value for the given pattern
 *
 * @returns byte containing the given pattern
 *
 */
static uint8_t get_pattern_value(unsigned int size, unsigned int offset,
			         uint8_t last_value, unsigned int pattern,
				 unsigned int mask = 0xFFFFFFFF)
{
	/* Write an increasing number */
	if (pattern == LINEAR)
		return offset;

	/* Write a decreasing number */
	if (pattern == LINEAR_REV)
		return (size - offset);

	/* Write a 1 on different positions in each byte (32 bit) */
	if (pattern == CIRCULAR)
		return ((last_value % 32) == 0 ? 0x00000001 : (last_value << 1));

	/* Write a 1 on different positions in each byte (16 bit) */
	if (pattern == CYCLE2_CIRCULAR)
		return ((last_value % 16) == 0 ? 0x00000001 : (last_value << 1));

	/* Write random values */
	if (pattern == RANDOM)
		return rand();

	/* Writes only some random bits */
	if (pattern == RANDOM_EXT)
		return (rand() & mask);

	cout << "Unknown pattern: " << pattern << endl;
	exit(EXIT_FAILURE);
}

/**
 *
 * Fills the given boards FPGA buffer of the given size at the
 * given offset with the given pattern
 *
 * @param copy Stores a copy of the written pattern in the given array
 *
 */
static void fill_fpga_buffer(Board *board, unsigned int offset, unsigned int size,
			    unsigned int pattern, unsigned int mask = 0xFFFFFFFF, uint32_t *copy = NULL)
{
	unsigned int i, temp = 0x00000001;

	for (i = 0; i < size; i++) {
		temp = get_pattern_value(size, i, temp, pattern, mask);
		if (copy != NULL)
			copy[i] = temp;
		//if (!use_fifo)
			board->write(FPGA_ADDR + offset + i, temp);
		//else board->writeFIFO(FIFO_ADDR + offset + i, temp);
	}
}


/**
 * The program can be called with -v or --verbose to enable verbose output.
 * -h or --help will print help.
 *
 * @return Returns success (0) if the test was passed without errors and
 * failure (1) if otherwise.
 *
 */
int main1() {

	/* seed random() */
	srand(time(NULL));

	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	ABB board(BOARD_NR);
	cout << "Starting threads for ABB Parallel test" << endl;

	verbose = true;
#if 0
	dma_read_thread(NULL);
#endif

	cout << "DMA read thread" << endl;
#if 0
	int iteration;
	int size = 40;
	int errors;

			/* Create a DMA buffer */
			DMABuffer buf(*board, (size * sizeof(int)), DMABuffer::USER);

			/* test the DMA read */
			errors += test_dma_read(board, buf, 0, 20, pattern);
#endif


	int size = 40;
	int i;
	DMABuffer buf(board, (size * sizeof(int)), DMABuffer::USER);
	int generated_pattern[size];
	int temp;
	int offset = 0;

	for (i = 0; i < size; i++)
		buf[i] = 0;

	board.writeDMA(FPGA_ADDR + offset, buf, size, 0, true, true);
	cout << "Verifying with PIO" << endl;
	for (i = 0; i < size; i++)
		if (board.read(FPGA_ADDR + offset + i) != 0) {
			cout << "ERROR detected" << endl;
			return 1;
		}

	/* Fill the DMA buffer with values */
	for (i = 0; i < size; i++) {
		buf[i] = generated_pattern[i] = temp = get_pattern_value(size, i, temp, LINEAR_REV);
		cout << "buf[i] = " << temp << endl;
	}

	cout << "Writing to card..." << endl;
	/* Make sure only one thread does a writeDMA at the moment */
	board.writeDMA(FPGA_ADDR, buf, size, 0, true, true);
	for (i = 0; i < size; i++) {
		//board->write(FPGA_ADDR + offset +i, buf[i]);
		if (board.read(FPGA_ADDR + offset + i) != buf[i]) {
			cerr << "PIO failed" << endl;
			return 1;
		}
	}
	//board->writeDMA(FPGA_ADDR + offset, buf, size, 0, true, true);
	cout << "Done." << endl;

	cout << "use_fifo = " << use_fifo << ", pio_only = " << use_pio_only << endl;

	/* Compare the values by reading without DMA */
	for (i = 0; i < size; i++) {
		uint8_t should = generated_pattern[i],
			is = board.read(FPGA_ADDR + offset + i);

		if (is == should)
			continue;

		printf("write: offset %x, value %x (card) != %x (buffer)\n", offset + i, is, should);
	}

}

int main2() {
	int size = 40;
	int i;
	ABB board(BOARD_NR);
	DMABuffer buf(board, (size * sizeof(int)), DMABuffer::USER);
	int generated_pattern[size];
	int temp;
	int offset = 0;

	for (i = 0; i < size; i++)
		buf[i] = 0;

	/* Fill the DMA buffer with values */
	for (i = 0; i < size; i++) {
		generated_pattern[i] = temp = get_pattern_value(size, i, temp, LINEAR_REV);
		cout << "buf[i] = " << temp << endl;
	}

	for (i = 0; i < size; i++) {
		buf[i] = 0xFF;
		cout << "buf[i] = " << i << endl;
	}

	cout << "reading DMA..." << endl;
	board.readDMA(FPGA_ADDR + offset, buf, size, 0, true, true);
	cout << "done" << endl;
	for (i = 0; i < size; i++) {
		uint8_t should = generated_pattern[i],
			is = buf[i];

		if (is == should)
			continue;

		printf("read: offset %x, value %x (DMA buffer) != %x (buffer)\n", offset + i, is, should);
	}
}
int main() {
	main1();
	main2();
}
