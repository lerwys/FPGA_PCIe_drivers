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
#include <stdint.h>

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
#define NLOOPS 		50

/** Minimum size of the buffer used for DMA */
static int MIN_SIZE = 1024;

/** Maximum size of the buffer used for DMA */
static int MAX_BLOCKRAM = 4096;


#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define FIFO_ADDR (0x0)

/** Converts the given double to megabytes */
#define TO_MiB(bytes) ((bytes) / (1024 * 1024))

/** Output verbose messages like which bytes failed in each test */
static bool verbose = false;

/** This option is only useful for debugging, it will disable DMA entirely. */
static bool use_pio_only = false;

static bool use_fifo = false;

/** The pattern to use for tests */
static int pattern = LINEAR_REV; //RANDOM;

/** Global instance of the board */
Board *board;

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
 */
static void fill_fpga_buffer(Board *board, unsigned int offset, unsigned int size,
			    unsigned int pattern, unsigned int mask = 0xFFFFFFFF)
{
	unsigned int i, temp = 0x00000001;

	for (i = 0; i < size; i++) {
		temp = get_pattern_value(size, i, temp, pattern, mask);
		cout << "Writing " << temp << " to FIFO_ADDR + offset + i = " << FIFO_ADDR + offset + i << endl;
		board->writeFIFO(FIFO_ADDR + offset + i, temp);
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
int main(int argc, char *argv[]) {
#if 0
	bool test_read = true;
	bool test_write = true;
	pthread_t read_thread, write_thread;
	unsigned int *retval_read, *retval_write;
	int c, option_index = 0;
	static struct option long_options[] = {
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "vhwrpf", long_options, &option_index)) != -1) {
		switch (c) {
			case 'v':
				verbose = true;
				break;
			case 'h':
				cout << "Syntax: " << argv[0] << " [-v] [-w] [-r] [-p] [-f]" << endl;
				cout << "-v\tEnable verbose output" << endl;
				cout << "-w\tTest only writes" << endl;
				cout << "-r\tTest only reads" << endl;
				cout << "-p\tUse PIO instead of DMA" << endl;
				exit(EXIT_SUCCESS);
				break;
			case 'w':
				test_read = false;
				test_write = true;
				cout << "PERFORMING ONLY WRITE TESTS" << endl;
				break;
			case 'r':
				test_read = true;
				test_write = false;
				cout << "PERFORMING ONLY READ TESTS" << endl;
				break;
			case 'p':
				use_pio_only = true;
				cout << "USING PIO ONLY" << endl;
				break;
			case 'f':
				use_fifo = true;
				cout << "USING FIFO INSTEAD OF BLOCKRAM" << endl;
				/* FIFO only has 64 dwords available */
				MIN_SIZE = 64;
				MAX_BLOCKRAM = 64;

				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}

	/* seed random() */
	srand(time(NULL));

	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);
	cout << "Starting FIFO test" << endl;

	cout << "FIFO status register is " << board->getReg(0x24)<< endl;

	if (test_write) {
		cout << "clearing the fifo by reading" << endl;
		board->setReg(0x24, 0x0A);
		mprace::util::Timer::wait(0.000000500);
		/* status reg == 0 means fifo still resetting */
		while (board->getReg(0x24) == 0)
			;

		cout << "Clearing time-out" << endl;
		board->setReg(0x1E, 0x0A);
		cout << "Cleared." << endl;

		if (use_pio_only) {
			uint32_t buf[64];
			cout << "Using PIO to fill the FIFO" << endl;
			unsigned int i, temp = 0x00000001;

			for (i = 0; i < 64; i++) {
				temp = get_pattern_value(64, i, temp, RANDOM);
				board->writeFIFO(FIFO_ADDR + i, temp);
				buf[i] = temp;
			}
                        cout << "FIFO status register is " << board->getReg(0x24)<< endl;

			for (int i = 0; i < 64; i++) {
				uint32_t read_board = board->readFIFO(0);
				if (read_board != buf[i]) {
					cout << "ERROR: value on the board is " << read_board << ", expected " << buf[i] << endl;
					cout << "FIFO status register is " << board->getReg(0x24) << endl;
					exit(1);
				}
			}
		} else {
			cout << "Creating DMABuffer of " << (64  * sizeof(int)) << " size" << endl;
			DMABuffer buf(*board, (64 * sizeof(int)), DMABuffer::USER);
//			DMABuffer buf(*board, (64 * sizeof(int)), DMABuffer::KERNEL);
			unsigned int i, temp = 0x00000001;

			for (i = 0; i < 64; i++) {
				temp = get_pattern_value(64, i, temp, RANDOM);
				buf[i] = temp;
				cout << "pattern val at " << i << " is " << temp << endl;
			}

			cout << "writeDMAFIFO(0, buf, 64)" << endl;
			board->writeDMAFIFO(0, buf, 64);
			cout << "DMA write done" << endl;

			int copy[64];
			for (i = 0; i < 64; i++)
				copy[i] = buf[i];

			//DMABuffer readbuf(*board, (64 * sizeof(int)), DMABuffer::USER);
			cout << "reading DMA" << endl;
			board->readDMAFIFO(0, buf, 64);
			for (i = 0; i < 64; i++) {
				cout << "read " << buf[i] << " vs. " << copy[i] << endl;
			}

		}

	}


	if (test_read) {
#endif
	        board = new ABB(BOARD_NR);
		for (int i = 0; i < 64; i++) {
			cout << "read " << board->readFIFO(0) << endl;
		}
		cout << "FIFO status register is " << board->getReg(0x24) << endl;
#if 0
		cout << "Clearing time-out" << endl;
		board->setReg(0x1E, 0x0A);
		cout << "Cleared." << endl;
#endif
		cout << "FIFO status register is " << board->getReg(0x24) << endl;

#if 0
	}
#endif
	return 0;
}
