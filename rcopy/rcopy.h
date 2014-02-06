/*
*  CPE 357-01 (Staley, Clint)
*  Winter 2012 
* 
*  Lab X
*  rcopy header
* 
*  Created Minh Nguyen on 11/5/12. Version 1.0.
*/ 

#include "networks.h"
#include "cpe464.h"

typedef enum State STATE;

enum State {
   DONE, FILENAME, RECV_DATA, FILE_OK, SELREJ
};

struct packet {
	int32_t seq_num;
	uint32_t length;
	uint8_t written;
	uint8_t data[MAX_LEN];
};

struct window {
	uint32_t window_size;
	Packet *window;
	uint32_t head;
};

Connection server;

/*
 * Sends file information to the server
 */
STATE filename(char *fname, int32_t buf_size, int32_t window_size, 
					char *server_machine);
/*
 * Receives data from the server and buffers it.
 * If possible, any data than can be written will be written
 */
STATE recv_data(FILE *output_file, Window *window, int32_t *expected_seq_num, 
					 int32_t *quit_seq_num);
/*
 *	Enter the selective reject state when a packet has been lost or had bits flipped
 *	and new data is still coming from the server.
 *	Buffer all data until the rejected packet has been received.
 */
STATE select_rej(FILE *output_file, Window *window, int32_t *expected_seq_num, 
					  int32_t quit_seq_num);
/*	
 *	Buffers data
 */
void buffer_data(int32_t seq_num, int32_t seq_num_ndx, int32_t data_len, 
					  uint8_t *data_buf, Window *window );
/*
 *	Writes data to file and slides window
 */
STATE write_packet(FILE *output_file, Window *window, int32_t *expected_seq_num, 
						int32_t quit_seq_num);
/*
 *	Checks the arguments of rcopy
 */
void check_args(int argc, char **argv);