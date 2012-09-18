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
#include <cstring>
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

/** If this flag is set to true, only one DMA buffer will be created and
 * re-used during the whole test. Otherwise, one DMA buffer is created for
 * each iteration. */
static bool use_same_buffer = false;

/** If this timestamp is set to something different to 0, the loop will not
 * run for X iterations, but as long as time() is lower or equal to the given
 * timestamp. This can be used to test for, say, 2 weeks. See date(1) for a
 * way to convert a human readable date to a timestamp. */
static time_t end_timestamp = 0;

/** This option is only useful for debugging, it will disable DMA entirely. */
static bool use_pio_only = false;

static bool use_fifo = false;

/** The pattern to use for tests */
static int pattern = RANDOM;

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
		if (!use_fifo)
			board->write(FPGA_ADDR + offset + i, temp);
		else board->writeFIFO(FIFO_ADDR + offset + i, temp);
	}
}


/**
 *
 * Tests a DMA read by first filling the DMA buffer with the given pattern and then
 * verifying that the buffer read via DMA is the same as when read via PIO.
 *
 * @param verbose_override If set to true, this function won't print anything
 * despite the value of verbose
 * @return The number of errors detected
 *
 */
static unsigned int test_dma_read(Board *board, DMABuffer &buf,
				  unsigned int offset, unsigned int size,
				  int pattern, bool verbose_override = false)
{
	unsigned int i;
	unsigned int errors = 0;
	mprace::util::Timer timer;
	uint32_t generated_pattern[size];

	if (verbose)
		cout << "Testing DMA read at " << offset << " with " << size << " bytes, verbose_override = " << verbose_override << endl;

	if (use_fifo) {
		/* Clear the timeout bit */
		board->setReg(TX_CTRL, CLEAR_TIMEOUT);
	}

	/* Only for blockram / PIO tests we can use fill_fpga_buffer */
	if (use_pio_only || !use_fifo)
		/* Fill the buffer on the FPGA to have some data to read */
		fill_fpga_buffer(board, offset, size, pattern, 0xFFFFFFFF, generated_pattern);
	else {
		unsigned int temp = 0x00000001;

		for (i = 0; i < size; i++)
			buf[i] = generated_pattern[i] = temp = get_pattern_value(size, i, temp, pattern, 0xFFFFFFFF);

		board->writeDMAFIFO(FIFO_ADDR + offset, buf, size);
	}

	/* Clear the DMA buffer */
	for (i = 0; i < size; i++)
		buf[i] = i;

	/* Make sure there is only one thread doing a readDMA at the moment */
	pthread_mutex_lock(&dma_read_mutex);

	if (!use_pio_only) {
		/* Read the DMA buffer */
		if (!use_fifo) {
			board->readDMA(FPGA_ADDR + offset, buf, size, 0, true, true);
		} else board->readDMAFIFO(FIFO_ADDR + offset, buf, size, 0, true, true);
	} else {
		for (i = 0; i < size; i++) {
			if (!use_fifo)
				buf[i] = board->read(FPGA_ADDR + offset + i);
			else buf[i] = board->readFIFO(FIFO_ADDR + offset + i);
		}
	}

	pthread_mutex_unlock(&dma_read_mutex);

	/* Compare the values by reading without DMA */
	for (i = 0; i < size; i++) {
		uint8_t should = buf[i],
			is = (!use_fifo ? board->read(FPGA_ADDR + offset + i) : generated_pattern[i]);

		if (is == should)
			continue;

		errors++;
		if (verbose)
			printf("read: offset %02x, value %02x (card) != %x (buffer)\n", offset + i, is, should);
	}

	if (verbose)
		cout << "Iteration done, " << errors << " errors" << endl;

	return errors;
}

/**
 * Tests a DMA write by first filling the FPGA buffer with the linear pattern, then
 * writing the given pattern to the board and verifying that it has been written
 * correctly by reading it with PIO again.
 *
 * @see test_dma_read
 * @see get_pattern_value
 * @return The number of errors detected
 *
 */
static unsigned int test_dma_write(Board *board, DMABuffer &buf,
				   unsigned int offset, unsigned int size, int pattern)
{
	unsigned int i, temp = 0x00000001;
	unsigned int errors = 0;
	mprace::util::Timer timer;
	uint32_t generated_pattern[size];

	if (verbose)
		cout << "Testing DMA write at " << offset << " with " << size << " bytes" << endl;

	if (use_fifo) {
		/* Clear the timeout bit */
		board->setReg(TX_CTRL, CLEAR_TIMEOUT);
	}

	/* Fill the buffer on the FPGA with sane defaults and make sure they
	   are written correctly */
	if ((errors = test_dma_read(board, buf, offset, size, LINEAR, true)) != 0)
		return errors;

	/* Fill the DMA buffer with values */
	for (i = 0; i < size; i++)
		buf[i] = generated_pattern[i] = temp = get_pattern_value(size, i, temp, pattern);

	/* Make sure only one thread does a writeDMA at the moment */
	pthread_mutex_lock(&dma_write_mutex);
	if (!use_pio_only) {
		/* Write the DMA buffer */
		if (!use_fifo)
			board->writeDMA(FPGA_ADDR + offset, buf, size, 0, true, true);
		else board->writeDMAFIFO(FIFO_ADDR + offset, buf, size, 0, true, true);
	} else {
		for (i = 0; i < size; i++) {
			if (!use_fifo)
				board->write(FPGA_ADDR + offset + i, buf[i]);
			else board->writeFIFO(FIFO_ADDR + offset + i, buf[i]);
		}
	}
	pthread_mutex_unlock(&dma_write_mutex);

	cout << "use_fifo = " << use_fifo << ", pio_only = " << use_pio_only << endl;
	if (use_fifo && !use_pio_only) {
		cout << "Reading back with DMA" << endl;
		/* We cannot mix DMA and PIO transfers, so we read back using DMA */
		board->readDMAFIFO(FIFO_ADDR + offset, buf, size, 0, true, true);
	}

	/* Compare the values by reading without DMA */
	for (i = 0; i < size; i++) {
		uint8_t should = generated_pattern[i],
			is = (!use_fifo ? board->read(FPGA_ADDR + offset + i) : buf[i]);

		if (is == should)
			continue;

		errors++;
		if (verbose)
			printf("write: offset %x, value %x (card) != %x (buffer)\n", offset + i, is, should);
	}

	if (verbose)
		cout << "Iteration done, " << errors << " errors" << endl;

	return errors;
}

/**
 * Tests DMA read performance by reading NLOOPS time and averaging the time
 * needed for that.
 *
 */
static void test_dma_read_performance(Board *board, DMABuffer &buf,
				      unsigned int offset, unsigned int size)
{
	unsigned int i;
	mprace::util::Timer timer;

	if (use_pio_only) {
		cout << "Skip DMA performance test because PIO mode is enabled" << endl;
		return;
	}

	timer.start();
	for (i = 0; i < NLOOPS; i++) {
		/* Read, but don't increment the offset on the FPGA, so
		   we don't go out of bounds (we read more often than
		   we reserved buffer space) for */

		pthread_mutex_lock(&dma_read_mutex);
		if (!use_fifo)
			board->readDMA(FPGA_ADDR + offset, buf, size, 0, false, true);
		else board->readDMAFIFO(FIFO_ADDR + offset, buf, size, 0, false, true);
		pthread_mutex_unlock(&dma_read_mutex);
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
 * Tests DMA write performance by writing NLOOPS time and averaging the time
 * needed for that.
 *
 */
static void test_dma_write_performance(Board *board, DMABuffer &buf,
				       unsigned int offset, unsigned int size)
{
	unsigned int i;
	mprace::util::Timer timer;

	if (use_fifo) {
		cout << "Skip DMA write performance test because working on FIFO" << endl;
		return;
	}

	if (use_pio_only) {
		cout << "Skip DMA performance test because PIO mode is enabled" << endl;
		return;
	}

	timer.start();
	for (i = 0; i < NLOOPS; i++) {
		/* Write, but don't increment the offset on the FPGA, so
		   we don't go out of bounds (we read more often than
		   we reserved buffer space) for */
		pthread_mutex_lock(&dma_write_mutex);
		if (!use_fifo)
			board->writeDMA(FPGA_ADDR + offset, buf, size, 0, false, true);
		else board->writeDMAFIFO(FIFO_ADDR + offset, buf, size, 0, false, true);
		pthread_mutex_unlock(&dma_write_mutex);
	}
	timer.stop();

	/* averaged time for a DMA read */
	double time = (timer.asSeconds() / NLOOPS);
	double performance;

	if (time < 1e-12)
		performance = 0;
	else performance = TO_MiB((size * sizeof(int)) / time);

	cout << "DMA write performance: " << performance << " MiB" << endl;
}

/**
 * This thread calls test_dma_read with the different buffer sizes repeatedly
 *
 */
static void *dma_read_thread(void *unused)
{
	unsigned int size = MIN_SIZE, iteration;
	/* "Global" error counter over all iterations this thread does.
	   It gets returned via pthread_exit, that's why we have to allocate it. */
	unsigned int *errors;
	DMABuffer *buf = NULL;

	errors = (unsigned int*)calloc(1, sizeof(unsigned int));

	cout << "DMA read thread" << endl;
	if (use_same_buffer)
		buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);

	for (iteration = 0;
		((end_timestamp > 0 && time(NULL) <= end_timestamp) ||
		(end_timestamp == 0 && iteration < NLOOPS));
		iteration++) {
		cerr << "Iteration " << iteration << ", running until " << end_timestamp << endl;
		fflush(stderr);
		for (size = MIN_SIZE;
				(use_same_buffer && size <= MIN_SIZE) ||
				(!use_same_buffer && size <= (MAX_BLOCKRAM / 2));
				size *= 2) {
			/* Create a DMA buffer */
			if (!use_same_buffer)
				buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);

			/* test the DMA read */
			*errors += test_dma_read(board, *buf, 0, size, pattern);

			/* measures the DMA read performance */
			test_dma_read_performance(board, *buf, 0, size);

			if (!use_same_buffer) {
				delete buf;
				buf = NULL;
			}
		}
	}

	pthread_exit((void*)errors);
}

/**
 * This thread calls test_dma_write with the different buffer sizes repeatedly
 *
 */
static void *dma_write_thread(void *unused)
{
	unsigned int size = MIN_SIZE, iteration;
	/* "Global" error counter over all iterations this thread does.
	   It gets returned via pthread_exit, that's why we have to allocate it. */
	unsigned int *errors;
	DMABuffer *buf = NULL;

	errors = (unsigned int*)calloc(1, sizeof(unsigned int));

	cout << "DMA write thread" << endl;
	if (use_same_buffer)
		buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);

	for (iteration = 0;
		((end_timestamp > 0 && time(NULL) <= end_timestamp) ||
		(end_timestamp == 0 && iteration < NLOOPS));
		iteration++) {
		cerr << "Iteration " << iteration << ", running until " << end_timestamp << endl;
		fflush(stderr);
		for (size = MIN_SIZE;
			(use_same_buffer && size <= MIN_SIZE) ||
			(!use_same_buffer && size <= (MAX_BLOCKRAM / 2));
			size *= 2) {
			cerr << "Size = " << size << "(min = " << MIN_SIZE << ")" << endl;
			/* Create a DMA buffer */
			if (!use_same_buffer)
				buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);

			/* test the DMA write */
			*errors += test_dma_write(board, *buf, (MAX_BLOCKRAM / 2), size, pattern);

			/* measures the DMA write performance */
			test_dma_write_performance(board, *buf, (MAX_BLOCKRAM / 2), size);

			if (!use_same_buffer) {
				delete buf;
				buf = NULL;
			}
		}
	}

	pthread_exit((void*)errors);
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
	bool test_read = true;
	bool test_write = true;
	pthread_t read_thread, write_thread;
	unsigned int *retval_read, *retval_write;
	int c, option_index = 0;
	static struct option long_options[] = {
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{"timestamp", 1, 0, 't'},
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "vhwrpft:s", long_options, &option_index)) != -1) {
		switch (c) {
			case 'v':
				verbose = true;
				break;
			case 'h':
				cout << "Syntax: " << argv[0] << " [-v] [-w] [-r] [-p] [-f] [-s] [-t unix timestamp]" << endl;
				cout << "-v\tEnable verbose output" << endl;
				cout << "-w\tTest only writes" << endl;
				cout << "-r\tTest only reads" << endl;
				cout << "-p\tUse PIO instead of DMA" << endl;
				cout << "-s\tUse the same buffer for all tests" << endl;
				cout << "-t\tSet end timestamp" << endl;
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
				MAX_BLOCKRAM = 1024;
				break;
			case 't':
				end_timestamp = atoi(optarg);
				cout << "End timestamp set to " << end_timestamp << endl;
				break;
			case 's':
				use_same_buffer = true;
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (test_read)
		pthread_mutex_init(&dma_read_mutex, NULL);
	if (test_write)
		pthread_mutex_init(&dma_write_mutex, NULL);

	/* seed random() */
	srand(time(NULL));

	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);
	cout << "Starting threads for ABB Parallel test" << endl;

	if (use_fifo) {
		cout << "Resetting FIFO..." << endl;
		board->setReg(0x24, 0x0A);
		mprace::util::Timer::wait(0.000000500);
		/* status reg == 0 means fifo still resetting */
		while (board->getReg(0x24) == 0)
			;
		cout << "FIFO status : " << board->getReg(0x24) << endl;
	}

	/* Create the threads which will run in parallel for some time */
	if (test_read)
		pthread_create(&read_thread, NULL, dma_read_thread, NULL);
	if (test_write)
		pthread_create(&write_thread, NULL, dma_write_thread, NULL);

	/* Wait for termination of both threads */
	if (test_read)
		pthread_join(read_thread, (void**)&retval_read);
	if (test_write)
		pthread_join(write_thread, (void**)&retval_write);

	if (test_read)
		cout << "Read thread returned " << *retval_read << " errors" << endl;
	if (test_write)
		cout << "Write thread returned " << *retval_write << " errors " << endl;

	bool retval = ((!test_read || *retval_read == 0) && (!test_write || *retval_write == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	if (retval != 0) {
		cout << "FIFO status : " << board->getReg(0x24) << endl;
		cout << "THERE HAVE BEEN ERRORS!" << endl;
		if (!verbose)
			cout << "Re-run this testcase with option -v to enable verbose output" << endl;
	}
	return retval;
}
