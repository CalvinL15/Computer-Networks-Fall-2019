#include "UDP_socket.h"
#include "opencv2/opencv.hpp"
#include <signal.h>

#define BUF_STR 1024

void setIP(char *dst, char *src) {
    if(strcmp(src, "0.0.0.0") == 0 || strcmp(src, "local") == 0
            || strcmp(src, "localhost")) {
        sscanf("127.0.0.1", "%s", dst);
    } else {
        sscanf(src, "%s", dst);
    }
}

int main(int argc, char *argv[]){
	int check = mkdir("sender_file", 0777);
	char agent_IP[50], sender_IP[50];
	int sender_port, recv_port, agent_port;
	if(argc != 5){
		fprintf(stderr,"用法: ./sender <agent IP> <sender_IP> <sender port> <agent port>\n");
        fprintf(stderr, "例如: ./sender local local 8887 8888\n");
        exit(1);
	}
	else{
		setIP(agent_IP, argv[1]);
		setIP(sender_IP, argv[2]);
    	sscanf(argv[3], "%d", &sender_port);
    	sscanf(argv[4], "%d", &agent_port);
	}
	int flag = 0;
	char act_com[5], act_fnm[BUF_STR];
	FILE *fp;
	while(flag == 0){
		fprintf(stderr,"Please give the input for the command and filename:\n");
		char command[100], filename[100];
		memset(&command, 0, sizeof(command));
		memset(&filename, 0, sizeof(filename));
		scanf("%s %s", command, filename);
		char pthname[BUF_STR] = "sender_file/";
		strcat(pthname, filename);
		fp = fopen(pthname, "rb");
		if(strcmp(command, "send") != 0 && strcmp(command, "play") != 0){
			fprintf(stderr, "Command not found.\n");
		}
		else if(fp == NULL){
			fprintf(stderr, "%s does not exist.\n", filename);
		//	fclose(fp);
		}
		else{
			flag = 1;
			strcpy(act_com, command);
			strcpy(act_fnm, pthname);
		}
	}
	fp = fopen(act_fnm, "rb");
	int fin = 0;
	//fprintf(stderr, "%d\n", sender_port);
	int socket_fd = create_socket(sender_port, sender_IP);
	struct sockaddr_in agent;
	set_addr(&agent, agent_IP, agent_port);
	
	struct timeval timeout;
	fd_set in_set;
	timeout.tv_sec = 1; //1 second
	timeout.tv_usec = 0;
	//FD_ZERO(&in_set);
	//FD_SET(socket_fd, &in_set);

	int threshold = 32;
	int windowsize = 1, result;
	segment s;
	s.head.seqNumber = 0, s.head.ackNumber = 0, s.head.fin = 0, s.head.syn = 0, s.head.ack = 0, s.head.length = 0;
	int smseq = 0, last_seq = -1;
	int tflag;
	int ack_seq_num = 0;
	if(strcmp(act_com, "send") == 0){
		F:
		while(1){
		//	memset(&s, 0, sizeof(s));
			s.head.ack = 0;
			s.head.ackNumber = ack_seq_num;
			int lower_bound = ack_seq_num + 1;
			int upper_bound = ack_seq_num + windowsize;
			for(int i = lower_bound; i<=upper_bound && fin == 0; i++){
				fseek(fp, (i-1)*1000, SEEK_SET);
				result = fread(s.data, 1, 1000, fp);
				if(result < 0){
					perror("file read error.\n");
					exit(1);
				}
				if(result == 0){
					last_seq = i-1;
					break;
				}
				s.head.syn = 0;
				s.head.seqNumber = i;
				s.head.length = result;
				if(send_packet(socket_fd, &s, &agent)){
					if(i > smseq)
						fprintf(stderr, "send\tdata\t#%d,\twinSize = %d\n", s.head.seqNumber, windowsize);	
					else fprintf(stderr, "resnd\tdata\t#%d,\twinSize = %d\n", s.head.seqNumber, windowsize);		
				}
			}
			if(smseq < upper_bound)
				smseq = upper_bound;
			int prev_winsize = windowsize;
			if(windowsize < threshold)
				windowsize *= 2;
			else windowsize += 2;
			while(s.head.ackNumber != upper_bound){
				if(s.head.ackNumber == last_seq && last_seq != -1){
					s.head.fin = 1;
					send_packet(socket_fd, &s, &agent);
					fprintf(stderr, "send\tfin\n");
					recv_packet(socket_fd, &s, &agent);
					s.head.ack = 0;
					fprintf(stderr, "recv\tfinack\n");
					fin = 1;
					break;
				}

				FD_ZERO(&in_set);
				FD_SET(socket_fd, &in_set);
				//receiving ACK & timeout
				timeout.tv_sec = 0; 
				timeout.tv_usec = 5000; 
				tflag = select(socket_fd+1, &in_set, NULL, NULL, &timeout);
				if(tflag == -1) {
					perror("socket reading failed\n");
					exit(1);
				} 
				else if(!tflag){	// Timeout
					threshold = max(prev_winsize/2, 1);
					windowsize = 1;
					fprintf(stderr, "time\tout\t\tthreshold = %d\n", threshold);
					break;
				}
				else{
					if(recv_packet(socket_fd, &s, &agent)){
						s.head.ack = 0;
						if(s.head.syn == 1){
							fprintf(stderr, "recv\tack\t#%d\n", s.head.seqNumber);
							if(s.head.seqNumber == ack_seq_num+1)
								ack_seq_num++;
						s.head.ackNumber = ack_seq_num;	
						}
					}
				}
			}
			if(s.head.fin == 1)
				break;
			if(fin == 1)
				break;
		}

	}
	else{
		while(1){
		//	memset(&s, 0, sizeof(s));
			s.head.ack = 0;
			s.head.ackNumber = ack_seq_num;
			int lower_bound = ack_seq_num + 1;
			int upper_bound = ack_seq_num + windowsize;
			for(int i = lower_bound; i<=upper_bound && fin == 0; i++){
				fseek(fp, (i-1)*1000, SEEK_SET);
				result = fread(s.data, 1, 1000, fp);
				if(result < 0){
					perror("file read error.\n");
					exit(1);
				}
				if(result == 0){
					last_seq = i-1;
					break;
				}
				s.head.syn = 0;
				s.head.seqNumber = i;
				s.head.length = result;
				if(send_packet(socket_fd, &s, &agent)){
					if(i > smseq)
						fprintf(stderr, "send\tdata\t#%d,\twinSize = %d\n", s.head.seqNumber, windowsize);	
					else fprintf(stderr, "resnd\tdata\t#%d,\twinSize = %d\n", s.head.seqNumber, windowsize);		
				}
			}
			if(smseq < upper_bound)
				smseq = upper_bound;
			int prev_winsize = windowsize;
			if(windowsize < threshold)
				windowsize *= 2;
			else windowsize += 2;
			while(s.head.ackNumber != upper_bound){
				if(s.head.ackNumber == last_seq && last_seq != -1){
					s.head.fin = 1;
					send_packet(socket_fd, &s, &agent);
					fprintf(stderr, "send\tfin\n");
					recv_packet(socket_fd, &s, &agent);
					s.head.ack = 0;
					fprintf(stderr, "recv\tfinack\n");
					fin = 1;
					break;
				}

				FD_ZERO(&in_set);
				FD_SET(socket_fd, &in_set);
				//receiving ACK & timeout
				timeout.tv_sec = 0; 
				timeout.tv_usec = 100; 
				tflag = select(socket_fd+1, &in_set, NULL, NULL, &timeout);
				if(tflag == -1) {
					perror("socket reading failed\n");
					exit(1);
				} 
				else if(!tflag){	// Timeout
					threshold = max(prev_winsize/2, 1);
					windowsize = 1;
					fprintf(stderr, "time\tout\t\tthreshold = %d\n", threshold);
					break;
				}
				else{
					if(recv_packet(socket_fd, &s, &agent)){
						s.head.ack = 0;
						if(s.head.syn == 1){
							fprintf(stderr, "recv\tack\t#%d\n", s.head.seqNumber);
							if(s.head.seqNumber == ack_seq_num+1)
								ack_seq_num++;
						s.head.ackNumber = ack_seq_num;	
						}
					}
				}
			}
			if(s.head.fin == 1)
				break;
			if(fin == 1)
				break;
		}
	}
	fclose(fp);
	return 0;
}