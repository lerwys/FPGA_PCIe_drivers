/**
 *
 * @file testIPCclient.cpp
 * @author Michael Stapelberg
 * @date 2009-04-19
 * @brief IPC test, client part. Puts messages of different priorities in the message queue.
 *
 * This is the counterpart for testIPCserver.cpp. This test sends different types of messages
 * to the server and waits for the replies.
 *
 * You should first start testIPCclient with the --fill parameter first, as this test sends 10 requests
 * which take 10 seconds to be replied.
 *
 * After starting it with --fill, start another instance with --info and see that the info requests
 * with higher priority gets answered as the fill test is still running.
 *
 */
#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <getopt.h>
#include "IPCshared.h"

using namespace boost::interprocess;
using namespace std;

/**
 * Opens a message queue with the given prefix plus a random suffix
 *
 * @param prefix Must be shorter than 10 characters to leave room for a useful random suffix.
 * @returns The name of the created queue
 *
 */
static char *open_message_queue(const char *prefix, size_t max_num_msg, size_t max_msg_size) {
	char *name = (char*)malloc(16 * sizeof(char));
	while (1) {
		memset(name, 0, 16);
		strcpy(name, prefix);
		while (strlen(name) < 15)
			name[strlen(name)] = (rand() % 26) + 97;

		cout << "Trying message queue " << name << endl;
		try {
			message_queue mq(create_only, name, max_num_msg, max_msg_size);
			break;
		}
		catch (interprocess_exception &ex) {
		}
	}
	return name;
}

/**
 * Sends one info request with higher priority to the IPC-server, waits for an answer and then prints
 * the dummy_message of the answer.
 *
 * @params mq The already opened shared message queue
 *
 */
static void test_info_requests(message_queue &mq) {
	struct mprace_request request;
	cout << "Creating queue for info answers" << endl;
	char *info_queue = open_message_queue("info-", 10, sizeof(struct mprace_info_answer));

	cout << "Sending request..." << endl;
	/* Send one request of high priority */
	request.id = 23;
	request.type = REQUEST_TYPE_GET_INFO;
	strcpy(request.answer_queue, info_queue);
	mq.send(&request, sizeof(struct mprace_request), 10);

	cout << "Receiving answers..." << endl;
	message_queue mq_info(open_only, info_queue);
	struct mprace_info_answer info_answer;
	size_t read_bytes;
	unsigned int priority;

	mq_info.receive(&info_answer, sizeof(struct mprace_info_answer), read_bytes, priority);

	cout << info_answer.dummy_message << endl;

	message_queue::remove(info_queue);
}

/**
 * Sends 10 fill requests and a buffer request to the IPC-server and waits for the replies.
 *
 * @params mq The already opened shared message queue
 *
 */
static void test_fill_requests(message_queue &mq) {
	cout << "Creating queue for fill answers" << endl;
	char *fill_queue = open_message_queue("fill-", 10, sizeof(struct mprace_fill_answer));
	char *buffer_queue = open_message_queue("buffer-", 10, sizeof(struct mprace_buffer_answer));

	cout << "Sending requests..." << endl;
	/* Send 10 requests of low priority */
	struct mprace_request request;

	for (int c = 0; c < 10; c++) {
		request.id = c;
		request.type = REQUEST_TYPE_FILL;
		strcpy(request.answer_queue, fill_queue);
		mq.send(&request, sizeof(struct mprace_request), 0);
	}

	request.id = 11;
	request.type = REQUEST_TYPE_GET_BUFFER;
	strcpy(request.answer_queue, buffer_queue);
	mq.send(&request, sizeof(struct mprace_request), 0);

	cout << "Receiving answers..." << endl;
	message_queue mq_fill(open_only, fill_queue);
	struct mprace_fill_answer fill_answer;
	size_t read_bytes;
	unsigned int priority;

	for (int c = 0; c < 10; c++) {
		mq_fill.receive(&fill_answer, sizeof(struct mprace_fill_answer), read_bytes, priority);

		cout << fill_answer.dummy_message << endl;
	}

	message_queue mq_buffer(open_only, buffer_queue);
	struct mprace_buffer_answer buffer_answer;

	mq_buffer.receive(&buffer_answer, sizeof(struct mprace_buffer_answer), read_bytes, priority);

	cout << "Dumping buffer..." << endl;
	for (int c = 0; c < 4096; c++) {
		if ((c % 16) == 0) {
			cout << endl << "Offset ";
			cout.width(8);
			cout.fill('0');
			cout << hex << c << ": ";
		}
		cout.width(2);
		cout << hex << (int)buffer_answer.data[c] << " ";
	}

	cout << endl;

	message_queue::remove(fill_queue);
	message_queue::remove(buffer_queue);
}

static void print_syntax(const char *name) {
	cout << "Syntax: " << name << " [--fill | --info]" << endl;
	cout << "You should first start the fill test, then start a second process with info test." << endl;
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	/* Initialize the random number generator */
	srand(time(NULL));

	try {
		cout << "Opening the message queue..." << endl;
		/* Open the message queue */
		message_queue mq(open_only, "mprace-queue");

		int c, option_index = 0;
		static struct option long_options[] = {
			{"fill", 0, 0, 'f'},
			{"info", 0, 0, 'i'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};

		while ((c = getopt_long(argc, argv, "fih", long_options, &option_index)) != -1) {
			switch (c) {
				case 'f':
					test_fill_requests(mq);
					return 0;
				case 'i':
					test_info_requests(mq);
					return 0;
				case 'h':
					print_syntax(argv[0]);
				default:
					exit(EXIT_FAILURE);
			}
		}

		print_syntax(argv[0]);
	}
	catch (interprocess_exception &ex) {
		std::cout << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
