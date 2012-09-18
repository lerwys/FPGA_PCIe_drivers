/**
 * This testcase ensures that DMA transfers using an offset correctly handle
 * the buffer (that is, they should not touch the buffers contents *before*
 * the offset). This is only problematic for machines with >= 4 GB of RAM
 * which are therefore using the SWIOTLB.
 *
 * The original problem was a missing sync of the buffer into the SWIOTLB
 * memory region.
 *
 * @file testOffset.cpp
 * @author Michael Stapelberg
 * @date 2010-01-25
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

enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR, FULL };

/** Minimum size of the buffer used for DMA */
static int MIN_SIZE = 32 * 4;  //2048;

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

/** Global instance of the board */
Board *board;

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
		temp = 0xEE;
		if (copy != NULL)
			copy[i] = temp;
                board->write(FPGA_ADDR + offset + i, temp);
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
				  unsigned int offset, unsigned int size)
{
	unsigned int i;
	unsigned int errors = 0;
	mprace::util::Timer timer;
	uint32_t generated_pattern[size];

        /* Fill the buffer on the FPGA to have some data to read */
        fill_fpga_buffer(board, offset, size, FULL, 0xFFFFFFFF, generated_pattern);

	/* Clear the DMA buffer */
	for (i = 0; i < size; i++)
		buf[i] = i;

        /* Read the DMA buffer */
        board->readDMA(FPGA_ADDR + offset, buf, size - 16, 16, true, true);

	/* See which values got touched */
	for (i = 0; i < 16; i++) {
		uint8_t should = i,
			is = buf[i],
                        card = board->read(FPGA_ADDR + offset +i) ;

		if (is == should)
			continue;

		errors++;
                printf("contents of buffer modified!: offset %02x, value %02x (card), %x (buffer), %x (should)\n", offset + i, card, is, should);
	}
	for (i = 16; i < size; i++) {
		uint8_t should = generated_pattern[i],
			is = buf[i],
                        card = board->read(FPGA_ADDR + offset +i) ;

		if (is == should)
			continue;

		errors++;
                printf("dma read delivered wrong data: offset %02x, value %02x (card), %x (buffer), %x (should)\n", offset + i, card, is, should);
	}


	return errors;
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

        size = MIN_SIZE;
        /* Create a DMA buffer */
        buf = new DMABuffer(*board, (size * sizeof(int)), DMABuffer::USER);

        /* test the DMA read */
        *errors += test_dma_read(board, *buf, 0, size);

        delete buf;
        buf = NULL;

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
	pthread_t read_thread;
	unsigned int *retval_read;

	if (test_read)
		pthread_mutex_init(&dma_read_mutex, NULL);

	/* seed random() */
	srand(time(NULL));

	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);
	cout << "Starting threads for ABB Parallel test" << endl;

	/* Create the threads which will run in parallel for some time */
	if (test_read)
		pthread_create(&read_thread, NULL, dma_read_thread, NULL);
	/* Wait for termination of both threads */
	if (test_read)
		pthread_join(read_thread, (void**)&retval_read);

	if (test_read)
		cout << "Read thread returned " << *retval_read << " errors" << endl;

	bool retval = ((!test_read || *retval_read == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	if (retval != 0) {
		cout << "THERE HAVE BEEN ERRORS!" << endl;
	}
	return retval;
}
