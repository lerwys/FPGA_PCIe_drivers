#ifndef _IPC_SHARED_H
#define _IPC_SHARED_H

enum request_type { REQUEST_TYPE_FILL, REQUEST_TYPE_GET_BUFFER, REQUEST_TYPE_GET_INFO };
enum TestPattern { RANDOM, RANDOM_EXT, LINEAR, LINEAR_REV, CIRCULAR, CYCLE2_CIRCULAR };

/**
 * General request sent by the client.
 *
 */
struct mprace_request {
	int id;			/**< ID of the request to distinguish, only interesting in this prototype. */
	char answer_queue[16];	/**< The name of the queue into which the server should store the answer. */
	int type;		/**< One of enum request_type */
};

/**
 * Answer for a fill request (REQUEST_TYPE_FILL)
 *
 */
struct mprace_fill_answer {
	char dummy_message[32];
};

/**
 * Answer for an info request (REQUEST_TYPE_GET_INFO)
 *
 */
struct mprace_info_answer {
	char dummy_message[32];
};

/**
 * Answer for a buffer request (REQUEST_TYPE_GET_BUFFER)
 *
 */
struct mprace_buffer_answer {
	uint8_t data[4096];
};

#endif
