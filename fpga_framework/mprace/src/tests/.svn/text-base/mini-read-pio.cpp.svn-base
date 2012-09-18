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

enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define FIFO_ADDR 	(0x0)
#define TX_CTRL		(0x1E)
#define CLEAR_TIMEOUT	(0x0A)



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

int main(int argc, char *argv[]) {

	/* seed random() */
	srand(time(NULL));

	/* Generally output trailing zeros when printing floats */
	cout.setf(ios::fixed, ios::floatfield);

	Board *board;
	board = new ABB(BOARD_NR);

	int i;
	int size = 17;

	uint32_t piobuf[size];
	uint32_t temp;

	for (i = 0; i < size; i++) {
		piobuf[i] = board->read(FPGA_ADDR + i);
		printf("%c", piobuf[i]);
	}
}
