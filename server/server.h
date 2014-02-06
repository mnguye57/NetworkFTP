/*
*  CPE 357-01 (Staley, Clint)
*  Winter 2012 
* 
*  Lab X
*  server header
* 
*  Created Minh Nguyen on 11/5/12. Version 1.0.
*/ 

#include "networks.h"
#include "cpe464.h"

typedef enum State STATE;

enum State {
   START, DONE, FILENAME, SEND_DATA, WAIT_ON_ACK, TIMEOUT_ON_ACK, BLOCK, WAIT_ON_QUIT  
};

struct packet {
	int32_t seq_num;
	uint32_t length;
	uint8_t data[MAX_LEN];
};

struct window {
	uint32_t window_size;
	Packet *window;
	int32_t head;
	int32_t new_rr_seq_num;
	int32_t old_rr_seq_num;
};

/*	
 *	Calls functions depending on what state the client is on
 */
void process_client(int32_t server_sk_num, uint8_t *buf, int32_t recv_len, 
						  Connection *client);
/*
 * Handles filename transfer at the beginning
 */
STATE filename(Connection *client, uint8_t *buf, int32_t recv_len, 
					int32_t *data_file, int32_t *buf_size, int32_t *window_size);
/*
 *	Reads data equal to the buffer size and sends it to the rcopy
 *	Then it processes all possible RRs/SREJs
 */
STATE send_data(Connection *client, uint8_t *packet, int32_t *packet_len, 
                int32_t data_file, int32_t buf_size, Window *window, int32_t *seq_num);
/*
 *	This state sends the lowest possible packet in the frame and processes 
 *	all incoming packets until the window is no longer closed
 */
STATE block(Connection *client, uint8_t *packet, int32_t buf_size, Window *window);
/*
 * When the file has been completely read and the window can no longer move,
 *	The server enters a stop and wait state, resending the END_OF_FILE packet
 *	until it receives an ACK and quits
 */
STATE wait_on_quit(Connection *client, uint8_t *packet, int32_t buf_size, 
						 Window *window, int32_t seq_num);
/*
 *	This function is called in other functions to process all possible RRs/SREJs
 */
STATE get_all_packets(Connection *client, uint8_t *packet, int32_t buf_size, 
							 Window *window, int32_t *send_count);