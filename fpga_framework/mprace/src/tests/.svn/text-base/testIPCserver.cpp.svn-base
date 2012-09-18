/**
 *
 * @file testIPCserver.cpp
 * @author Michael Stapelberg
 * @date 2009-04-19
 * @brief IPC test, server part. Reads messages from the message queue and displays their priority.
 *
 * This is only a prototype to demonstrate that this design actually works.
 *
 * Basically, there is one shared queue between all processes, the request queue. The server
 * processes requests, sorting first after priority, then in the order in which the messages arrived.
 *
 * For each request, the queue in which the server will put the answer in, can be specified. This is
 * necessary/an advantage for the following reasons:
 *
 * - There are different types of requests and especially answers. Chosing different queues
 *   guarantees that there isn't a single queue which gets clogged up quickly.
 *
 * - The client originating the request does not have to care about different message types
 *   when polling for the answer, as it will only get appropriate messages.
 *
 *
 */
#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>
#include <mprace/Board.h>
#include <mprace/DMABuffer.h>
#include <mprace/ABB.h>
#include <mprace/util/Timer.h>

#include "IPCshared.h"

/** Number of times the DMA read or write will be done to get decent performance
   measurement values */
#define NLOOPS 		50

/** Minimum size of the buffer used for DMA */
#define MIN_SIZE 	1024

/** Maximum size of the buffer used for DMA */
#define MAX_BLOCKRAM 	4096

#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

/** Converts the given double to megabytes */
#define TO_MiB(bytes) ((bytes) / (1024 * 1024))

using namespace boost::interprocess;
using namespace mprace;
using namespace std;

Board *abb_board;

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

static void fill_fpga_buffer(Board *board, unsigned int offset, unsigned int size,
			    unsigned int pattern, unsigned int mask = 0xFFFFFFFF)
{
	unsigned int i, temp = 0x00000001;

	for (i = 0; i < size; i++) {
		temp = get_pattern_value(size, i, temp, pattern, mask);
		board->write(FPGA_ADDR + offset + i, temp);
	}
}


void read_from_message_queue(message_queue &mq) {
	size_t read_bytes;
	unsigned int priority;
	struct mprace_request request;

	static int id_counter = 0;

	cout << "Reading..." << endl;

	mq.receive(&request, sizeof(struct mprace_request), read_bytes, priority);
	if (read_bytes != sizeof(struct mprace_request)) {
		cout << "Invalid request, only " << read_bytes << " of " << sizeof(struct mprace_request) << " read" << endl;
		exit(1);
	}

	cout << "priority = " << priority << endl;
	cout << "request.id = " << request.id << endl;
	cout << "request.type = " << request.type << endl;
	cout << "request.answer_queue = " << request.answer_queue << endl;
	switch (request.type) {
		case REQUEST_TYPE_FILL:
		{
			struct mprace_fill_answer answer;
			strcpy(answer.dummy_message, "Status: 23");
			message_queue answer_queue(open_only, request.answer_queue);

			cout << "Filling the FPGA buffer..." << endl;
			fill_fpga_buffer(abb_board, 0, MAX_BLOCKRAM, LINEAR);

			answer_queue.send(&answer, sizeof(struct mprace_fill_answer), 0);
			break;
		}
		case REQUEST_TYPE_GET_INFO:
		{
			struct mprace_info_answer answer;
			strcpy(answer.dummy_message, "Status: DONT PANIC");
			message_queue answer_queue(open_only, request.answer_queue);
			cout << "Answering info request (control application)" << endl;
			answer_queue.send(&answer, sizeof(struct mprace_info_answer), 0);
			break;
		}
		case REQUEST_TYPE_GET_BUFFER:
		{
			struct mprace_buffer_answer answer;
			message_queue answer_queue(open_only, request.answer_queue);
			memset(answer.data, 1, sizeof(answer.data));

			cout << "Reading DMA buffer" << endl;
			DMABuffer buf(*abb_board, (MAX_BLOCKRAM * sizeof(int)), DMABuffer::USER);

			abb_board->readDMA(FPGA_ADDR, buf, MAX_BLOCKRAM, 0, true, true);

			for (int c = 0; c < MAX_BLOCKRAM; c++)
				answer.data[c] = buf[c];

			answer_queue.send(&answer, sizeof(struct mprace_buffer_answer), 0);
			break;
		}
		default:
			cout << "ERROR: Request type " << request.type << " not yet implemented." << endl;
			break;
	}
	if (request.id == 23 && id_counter < 10) {
		cout << "TEST PASSED: Message of higher priority arrived earlier correctly." << endl;
	}

	/* Save the ID of the last request */
	id_counter = request.id;

	/* Simulate some time to process the request */
	sleep(1);
}

int main() {

	abb_board = new ABB(BOARD_NR);

	/* Retry upon exception. This process needs to run very stable */
	while (1) {
		try {
			/* Cleanup the queue if it already exists when we're starting */
			message_queue::remove("mprace-queue");

			/* Create a new message queue with max. 100 messages with the size of an mprace_request each */
			message_queue mq(create_only, "mprace-queue", 100, sizeof(struct mprace_request));

			/* Handle requests */
			while (1) {
				read_from_message_queue(mq);
			}

		}
		catch (interprocess_exception &ex){
			std::cout << ex.what() << std::endl;
			return 1;
		}
	}
	message_queue::remove("message_queue");
	return 0;
}
