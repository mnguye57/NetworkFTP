/*
 *  rcopy
 *  To run: rcopy fromFile toFile bufferSize errorRate windowSize serverMachine serverPort
 */ 

#include "rcopy.h"

int main (int argc, char *argv[]) {
   FILE *output_file;
   int32_t select_count = 0;
   STATE state = FILENAME;
	int32_t expected_seq_num = START_SEQ_NUM;
	int32_t quit_seq_num = -1;
	
   check_args(argc, argv);
   
	Packet dataWindow[atoi(argv[5])];
	Window window;
	
	window.window_size = atoi(argv[5]);
	window.window = dataWindow;
	window.head = 0;
	
   sendtoErr_init(atof(argv[4]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
   
   state = FILENAME;
   
   while (state != DONE) {
      switch (state) {
         case FILENAME:
            /* Ever time we try to start/restart a connection get a new socket */
            if(udp_client_setup(argv[6], atoi(argv[7]), &server) < 0)
               exit(-1);   /*if host is not found*/
            
            state = filename(argv[1], atoi(argv[3]), atoi(argv[5]), argv[6]);
            
            /* if no response from server then repeat sending filename 
				 *	(close socket) so you can open another */
            if (state == FILENAME)
               close(server.sk_num);
            
            select_count++;
            if(select_count > 9) {
               printf("Server unreachable, client terminating\n");
               state = DONE;
            }
            
            break;
            
         case FILE_OK:
            select_count = 0;
            
            if ((output_file = fopen(argv[2], "w")) == NULL) {
               perror("File open");
               state = DONE;
            }
            else {
               state = RECV_DATA;
            }
            
            break;
            
         case RECV_DATA:
            state = recv_data(output_file, &window, &expected_seq_num, &quit_seq_num);
            break;
            
         case DONE:
				
            break;
				
			case SELREJ:
				state = select_rej(output_file, &window, &expected_seq_num, quit_seq_num);
				break;
            
         default:
            printf("ERROR - in default state\n");
            break;
      }
   }
   
   return 0;
}

STATE filename(char *fname, int32_t buf_size, int32_t window_size, 
					char *server_machine) {
   uint8_t packet[MAX_LEN];
   uint8_t buf[MAX_LEN];
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t fname_len = strlen(fname) + 1;
   int32_t recv_check = 0;
   
	buf_size = htonl(buf_size);
	window_size = htonl(window_size);
	
   memcpy(buf, &buf_size, 4);
	memcpy(&buf[4], &window_size, 4);
   memcpy(&buf[8], fname, fname_len);
   
   send_buf(buf, fname_len + 8, &server, FNAME, 0, packet);
   
   if (select_call(server.sk_num, 1, 0, NOT_NULL) == 1) {
      recv_check = recv_buf(packet, MAX_LEN, server.sk_num, &server, &flag, &seq_num);
      
      /*check for bit flip... if so send the file name again*/
      if (recv_check == CRC_ERROR) {
         return FILENAME;
      }
      
      if (flag == FNAME_BAD) {
         printf("Error %s does not exist on %s\n", fname, server_machine);
         return (DONE);
      }
      
      return (FILE_OK);
   }
   
   return (FILENAME);
}

STATE recv_data(FILE *output_file, Window *window, int32_t *expected_seq_num, 
					 int32_t *quit_seq_num) {
   int32_t seq_num = 0;
   uint8_t flag = 0;
   int32_t data_len = 0;
   uint8_t data_buf[MAX_LEN];
   uint8_t packet[MAX_LEN];
	int32_t seq_num_ndx;
   
   if (select_call(server.sk_num, 10, 0, NOT_NULL) == 0) {
      printf("Timeout after 10 seconds, client done.\n");
      return DONE;
   }
   
   data_len = recv_buf(data_buf, MAX_LEN, server.sk_num, &server, &flag,  &seq_num);
   seq_num_ndx = (seq_num-1) % window->window_size;
	
   /* go back to recv_data if there is a crc error (don't send ack, dont' write data)*/
   if (data_len == CRC_ERROR) {
		return RECV_DATA;
   }
   
   if (flag == END_OF_FILE) {
      *quit_seq_num = seq_num;
   }
	
   if (seq_num == *expected_seq_num) {
		/* write data and send RR */
		buffer_data(seq_num, seq_num_ndx, data_len, data_buf, window);
		
		if (write_packet(output_file, window, expected_seq_num, *quit_seq_num) == DONE) {
			return DONE;
		}
	}
   else if (seq_num > *expected_seq_num) {
		buffer_data(seq_num, seq_num_ndx, data_len, data_buf, window);
		
      //handle out of order packet
      //Enter selective reject state
		send_buf(packet, 1, &server, SREJ, *expected_seq_num, packet);
		return SELREJ;
   }
	else {
		send_buf(packet, 1, &server, RR, (*expected_seq_num), packet);
		return RECV_DATA;
	}

   return RECV_DATA;
}

STATE select_rej(FILE *output_file, Window *window, int32_t *expected_seq_num, 
					  int32_t quit_seq_num) {
	int32_t seq_num = 0;
   uint8_t flag = 0;
   int32_t data_len = 0;
   uint8_t data_buf[MAX_LEN];
   uint8_t packet[MAX_LEN];
	int32_t seq_num_ndx;
   
   if (select_call(server.sk_num, 10, 0, NOT_NULL) == 0) {
      printf("Timeout after 10 seconds, client done.\n");
      return DONE;
   }
	
   data_len = recv_buf(data_buf, MAX_LEN, server.sk_num, &server, &flag,  &seq_num);
   seq_num_ndx = (seq_num-1) % window->window_size;
	
   /* restart selective reject on crc error*/
   if (data_len == CRC_ERROR) {
		return SELREJ;
   }
	
	buffer_data(seq_num, seq_num_ndx, data_len, data_buf, window);
	
	if (seq_num == *expected_seq_num) {
		/* send RR */
		while (window->window[window->head].written == 0 && 
				 window->window[window->head].seq_num == *expected_seq_num) {
			if (write_packet(output_file, window, expected_seq_num, quit_seq_num) == DONE) {
				return DONE;
			}
		}
		return RECV_DATA;
	}
	
	send_buf(packet, 1, &server, SREJ, *expected_seq_num, packet);

	return SELREJ;
}

void buffer_data(int32_t seq_num, int32_t seq_num_ndx, int32_t data_len, 
					  uint8_t *data_buf, Window *window ) {
	//buffers incoming data
	memcpy(window->window[seq_num_ndx].data, data_buf, data_len);
	window->window[seq_num_ndx].length = data_len;
	window->window[seq_num_ndx].seq_num = seq_num;
	window->window[seq_num_ndx].written = 0;
}

STATE write_packet(FILE *output_file, Window *window, int32_t *expected_seq_num, 
						int32_t quit_seq_num) {
	uint8_t packet[MAX_LEN];
	
	//end program if quit file is last file to be written
	if (quit_seq_num == window->window[window->head].seq_num) {
		send_buf(packet, 1, &server, RR, quit_seq_num, packet);
		return DONE;
	}
	
	(*expected_seq_num)++;
	fwrite(window->window[window->head].data, sizeof(uint8_t), 
			 window->window[window->head].length, output_file);
	window->window[window->head].written = 1;
	send_buf(packet, 1, &server, RR, window->window[window->head].seq_num + 1, packet);
	
	//slide window
	window->head = (window->head + 1) % window->window_size;
	
	return RECV_DATA;
}

void check_args(int argc, char **argv) {
   if (argc != 8) {
      printf("Usage %s fromFile toFile buffer_size error_rate window_size hostname port\n", argv[0]);
      exit(-1);
   }
   
   if (strlen(argv[1]) > 1000) {
      printf("FROM filename to long needs to be less than 1000 and is: %d\n", strlen(argv[1]));
      exit(-1);
   }
   
   if (strlen(argv[2]) > 1000) {
      printf("TO filename to long needs to be less than 1000 and is: %d\n", strlen(argv[2]));
      exit(-1);
   }
   
   if (atoi(argv[3]) < 400 || atoi(argv[3]) > 1400) {
      printf("Buffer size needs to be between 400 and 1400 and is: %d\n", atoi(argv[3]));
      exit(-1);
   }
   
   if (atoi(argv[4]) < 0 || atoi(argv[4]) >= 1) {
      printf("Error rate needs to be between 0 and less than 1 and is: %d\n", atoi(argv[4]));
      exit(-1);
   }
}