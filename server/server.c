/*
 * Works with Rcopy, sends a requested file
 * To run: server errorRate
 */

#include "server.h"

int main (int argc, char *argv[]) {
   int32_t server_sk_num = 0;
   pid_t pid = 0;
   int status = 0;
   uint8_t buf[MAX_LEN];
   Connection client;
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t recv_len = 0;
	
   if (argc != 2) {
      printf("Usage %s error_rate\n", argv[0]);
      exit(-1);
   }
   
   sendtoErr_init(atof(argv[1]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
   
   /* set up the main server port*/
   server_sk_num = udp_server();
   
   while (1) {
      if (select_call(server_sk_num, 1, 0, NOT_NULL) == 1) {
         recv_len = recv_buf(buf, MAX_LEN, server_sk_num, &client, &flag, &seq_num);
         if (recv_len != CRC_ERROR) {
            /* fork will go here*/
            if ((pid = fork()) < 0) {
               perror("fork");
               exit(-1);
            }
            
            //process child
            if (pid == 0) {
               process_client(server_sk_num, buf, recv_len, &client);
               exit(0);
            }
         }
         
         //check to see if any children quit
         while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("processed wait\n");
         }
      }
   }
   
   return 0;
}

void process_client(int32_t server_sk_num, uint8_t *buf, int32_t recv_len, 
						  Connection *client) {
   STATE state = START;
   int32_t data_file = 0;
   int32_t packet_len = 0;
   uint8_t packet[MAX_LEN];
   int32_t buf_size = 0;
	int32_t window_size = 0;
   int32_t seq_num = START_SEQ_NUM;
	static Window window;
	window.head = 0;
	window.new_rr_seq_num = 1;
	window.old_rr_seq_num = 0;
   
   while (state != DONE) {
      switch (state) {
         case START:
            state = FILENAME;
            break;
            
         case FILENAME:
            //seq_num = 1;
            state = filename(client, buf, recv_len, &data_file, &buf_size, &window_size);
				window.window_size = window_size;
				window.window = (Packet *)malloc(window_size * sizeof(Packet));
            break;
				
         case SEND_DATA:
				if (seq_num >= window.old_rr_seq_num + window.window_size) {
					state = BLOCK;
				}
            else {
					state = send_data(client, packet, &packet_len, data_file, buf_size, 
											&window, &seq_num);
				}
            break;
				
			case BLOCK:				
				state = block(client, packet, buf_size, &window);
				break;
				
			case WAIT_ON_QUIT:
				state = wait_on_quit(client, packet, buf_size, &window, seq_num);
				break;
				
         case DONE:
				free(window.window);
            break;
				
         default:
            printf("In default and you should not be here!!!!\n");
            state = DONE;
            break;
      }
   }
}

STATE filename(Connection *client, uint8_t *buf, int32_t recv_len, 
					int32_t *data_file, int32_t *buf_size, int32_t *window_size) {
   uint8_t response[1];
   char fname[MAX_LEN];
   
   memcpy(buf_size, buf, 4);
	memcpy(window_size, &buf[4], 4);
   memcpy(fname, &buf[8], recv_len - 8);
	
	*buf_size = ntohl(*buf_size);
	*window_size = ntohl(*window_size);
   
   /* Create client socket to allow for processing this particular client */
   
   if ((client->sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("filename, open client socket");
      exit(-1);
   }
	
   if (((*data_file) = open(fname, O_RDONLY)) < 0) {
      send_buf(response, 0, client, FNAME_BAD, 0, buf);
      return DONE;
   }
   else {
      send_buf(response, 0, client, FNAME_OK, 0, buf);
      return SEND_DATA;
   }
}

STATE send_data(Connection *client, uint8_t *packet, int32_t *packet_len, 
                int32_t data_file, int32_t buf_size, Window *window, int32_t *seq_num) {
   uint8_t *buf = window->window[(*seq_num) % window->window_size].data;
   int32_t len_read = 0;
	int send_count = 0;
	
	len_read = read(data_file, buf, buf_size);
   window->window[(*seq_num) % window->window_size].seq_num = *seq_num;
	window->window[(*seq_num) % window->window_size].length = len_read;
	
   switch (len_read) {
      case -1:
         perror("send_data, read error");
         return DONE;
         break;
			
      case 0:
         (*packet_len) = send_buf(buf, 1, client, END_OF_FILE, *seq_num, packet);
         printf("%d File reading complete.\n", *seq_num);
         return WAIT_ON_QUIT;;
         break;
			
      default:
         (*packet_len) = send_buf(buf, len_read, client, DATA, *seq_num, packet);
         (*seq_num)++;
			
			get_all_packets(client, packet, buf_size, window, &send_count);

         return SEND_DATA;
         break;
   }
}

STATE wait_on_quit(Connection *client, uint8_t *packet, int32_t buf_size, 
						 Window *window, int32_t seq_num) {
	static int32_t send_count = 0;
	int32_t recv_len = 0;
	uint8_t rrBuf[buf_size];
	uint8_t flag = 0;
	int32_t lost_packet_ndx = 0;
	Packet *lost_packet;
	uint8_t *buf = window->window[seq_num % window->window_size].data;
	
	if (send_count < 10) {
		send_buf(buf, 1, client, END_OF_FILE, seq_num, packet);
		send_count++;
	}
	else {
		return DONE;
	}
	
	if (select_call(client->sk_num, 1, 0, NOT_NULL) == 1) {
		while (select_call(client->sk_num, 0, 0, NOT_NULL) == 1) {
			recv_len = recv_buf(rrBuf, buf_size, client->sk_num, client, &flag, 
									  &(window->new_rr_seq_num));
			if (recv_len != CRC_ERROR) {
				switch (flag) {
					case RR:
						if (window->new_rr_seq_num == seq_num) {
							printf("File Transfer Complete.\n");
							return DONE;
						}
						send_count = 0;
						break;
						
					case SREJ:
						lost_packet_ndx = window->new_rr_seq_num % window->window_size;
						lost_packet = &(window->window[lost_packet_ndx]);
						send_buf(lost_packet->data, lost_packet->length, client, RESDATA, 
									lost_packet->seq_num, packet);
						send_count = 0;
						break;
						
					default:
						break;
				}
			}
			else {
				return WAIT_ON_QUIT;
			}
			
		}
	}
	
	return WAIT_ON_QUIT;
}

STATE block(Connection *client, uint8_t *packet, int32_t buf_size, Window *window) {
	uint32_t window_head = window->head;
	Packet *head_packet = &(window->window[window_head]);
	static int32_t send_count = 0;
	
	if (send_count < 10) {
		send_buf(head_packet->data, head_packet->length, client, RESDATA, 
					head_packet->seq_num, packet);
		send_count++;
	}
	else {
		send_count = 0;
		return DONE;
	}
	
	if (select_call(client->sk_num, 1, 0, NOT_NULL) == 1) {
		if(get_all_packets(client, packet, buf_size, window, &send_count) == BLOCK) {
			return BLOCK;
		}
	}
	return SEND_DATA;
}

STATE get_all_packets(Connection *client, uint8_t *packet, int32_t buf_size, Window *window, int32_t *send_count) {
	int32_t lost_packet_ndx = 0;
	Packet *lost_packet;
	int32_t recv_len = 0;
	uint8_t rrBuf[buf_size];
	uint8_t flag = 0;
	
	while (select_call(client->sk_num, 0, 0, NOT_NULL) == 1) {
		recv_len = recv_buf(rrBuf, buf_size, client->sk_num, client, &flag, 
								  &(window->new_rr_seq_num));
		if (recv_len != CRC_ERROR) {
			switch (flag) {
				case RR:
					if (window->new_rr_seq_num > window->old_rr_seq_num) {
						window->head = (window->new_rr_seq_num) % window->window_size;
						window->old_rr_seq_num = window->new_rr_seq_num;
					}
					(*send_count) = 0;
					break;
					
				case SREJ:
					lost_packet_ndx = window->new_rr_seq_num % window->window_size;
					lost_packet = &(window->window[lost_packet_ndx]);
					send_buf(lost_packet->data, lost_packet->length, client, RESDATA, 
								lost_packet->seq_num, packet);
					(*send_count) = 0;
					break;
					
				default:
					break;
			}
		}
		else {
			return BLOCK;
		}
	}
	
	return SEND_DATA;
}