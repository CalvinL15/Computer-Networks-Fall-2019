#include "UDP_socket.h"
#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

#define BUF_SIZE 128

void setIP(char *dst, char *src) {
    if(strcmp(src, "0.0.0.0") == 0 || strcmp(src, "local") == 0
            || strcmp(src, "localhost")) {
        sscanf("127.0.0.1", "%s", dst);
    } else {
        sscanf(src, "%s", dst);
    }
}

int main(int argc, char* argv[]){
	int check = mkdir("receiver_file", 0777);
	char receiver_IP[50], agent_IP[50];
	int sender_port, agent_port, recv_port;
	int mode;
	char filename[100];
	if(argc != 7){
        fprintf(stderr,"用法: ./receiver <agent IP> <receiver IP> <agent port> <recv port> <filename> <mode>\n");
        //mode = 0 if send file, 1 if play video.
        fprintf(stderr, "例如: ./receiver local local 8887 8888 output 0\n");
        exit(1);
    }
    else{
    	setIP(agent_IP, argv[1]);
    	setIP(receiver_IP, argv[2]);
    	sscanf(argv[3], "%d", &agent_port);
    	sscanf(argv[4], "%d", &recv_port);
    	sscanf(argv[5], "%s", filename);
    	sscanf(argv[6], "%d", &mode);
    }
    char pthname[1000] = "receiver_file/";
    strcat(pthname, filename);
    FILE *fp = fopen(pthname, "wb");
    if(fp == NULL){
    	perror("file open error.\n");
    	exit(1);
    }
    int socket_fd = create_socket(recv_port, receiver_IP);
    struct sockaddr_in agent;
	set_addr(&agent, agent_IP, agent_port);

	segment buffer[BUF_SIZE];
	segment s;
	int seq_ack_number = 0;
	s.head.seqNumber = 0;
	int n = 0;
	while(1){
		s.head.ackNumber = seq_ack_number;
		if(recv_packet(socket_fd, &s, &agent)){
			if(n == BUF_SIZE){
				for(int i = 0; i < BUF_SIZE; i++)
					fwrite(buffer[i].data, 1, buffer[i].head.length, fp);
				fprintf(stderr, "drop\tdata\t#%d\n", s.head.seqNumber);	
				s.head.seqNumber = seq_ack_number;
				s.head.ack = 1;
				s.head.syn = 1;
				if(send_packet(socket_fd, &s, &agent));
					fprintf(stderr, "send\tack\t#%d\n", s.head.seqNumber);
			//	}
				fprintf(stderr, "flush.\n");
				n = 0;
			}
			else if(s.head.fin == 1){
				s.head.ack = 1;
				send_packet(socket_fd, &s, &agent);
				fprintf(stderr, "recv\tfin\n");
				fprintf(stderr, "send\tfinack\n");
				break;
			}
			else if(s.head.syn == 0){
				s.head.syn = 1;
				if(s.head.seqNumber == seq_ack_number + 1){
					seq_ack_number++;
					s.head.ackNumber = seq_ack_number;
				//	s.head.ackNumber++;
					fprintf(stderr, "recv\tdata\t#%d\n", seq_ack_number);
					for(int i = 0; i < s.head.length; i++)
						buffer[n].data[i] = s.data[i];
					buffer[n].head.length = s.head.length;
					n++;
				}
				else{
					fprintf(stderr, "drop\tdata\t#%d\n", s.head.seqNumber);
					s.head.seqNumber = seq_ack_number;
				}
				s.head.ack = 1;
				if(send_packet(socket_fd, &s, &agent));
					fprintf(stderr, "send\tack\t#%d\n", s.head.seqNumber);
				//else s.head.ack = 0;
			}
		}
	}
	fprintf(stderr, "flush.\n");
	for(int i = 0; i < n; i++)
		fwrite(buffer[i].data, 1, buffer[i].head.length, fp);
	if(mode == 1){
		Mat imgrecv;
		VideoCapture cap(pthname);
	    int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	    int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	    imgrecv = Mat::zeros(height, width, CV_8UC3);
	    if(!imgrecv.isContinuous()){
	         imgrecv = imgrecv.clone();
	    }
		while(1){
			cap >> imgrecv;
			int imgsize = imgrecv.total() * imgrecv.elemSize();
			imshow("Video", imgrecv);
			 char c = (char)waitKey(33.3333);
        	if(c==27)
                break;
		}
		cap.release();
		destroyAllWindows();
	}

	fclose(fp);
return 0;
}
