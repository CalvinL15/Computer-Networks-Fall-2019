#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <limits.h>
#include "opencv2/opencv.hpp"
#include <signal.h>

using namespace cv;

#define BUFF_SIZE 1024
#define MAX_CLIENT 50
#define MAX_LINE 2048

typedef long long int LLI;
//signal(SIGPIPE, SIG_IGN);
LLI findsize(char *filename){
	FILE *fp = fopen(filename, "r");
	fseek(fp, 0LL, SEEK_END);
	LLI res = ftell(fp);
	fclose(fp);
	return res;	
}


void ls_function(const char *filename, int socket){
	DIR *d = opendir(filename);
	
	//if(d == NULL) return;
	struct dirent *dir;
	//char tosend[MAXLINE];
	//memset(&tosend, 0, sizeof(tosend));
	int snd = 0;
	if(d){
		while((dir = readdir(d)) != NULL){
			//if(dir->d_type == DT_REG){
				int len = strlen(dir->d_name);
				snd = send(socket, dir->d_name, len, 0);
				snd = send(socket, "\n", 1, 0);
			//}	
		}
		closedir(d);
	}
	
}

void shad(int sd, char Message[BUFF_SIZE]){
	//fprintf(stderr, "%s", Message);
						char namefile[BUFF_SIZE];
						memset(&namefile, 0, sizeof(namefile));
						strcpy(namefile, Message+5);
						//fprintf(stderr, "%s", namefile);
						char bufbuf[BUFF_SIZE] = "server_file/";
						strcat(bufbuf, namefile);
						FILE *fpr = fopen(bufbuf, "rb");
						int len = strlen(bufbuf);
						char tosnd[BUFF_SIZE];
						char fileformat[5];
						memset(&fileformat, 0, sizeof(fileformat));
						memset(&tosnd, 0, sizeof(tosnd));
						strcpy(fileformat, bufbuf+len-4);
						if(fpr == NULL) sprintf(tosnd, "DNE%s", namefile);
						else if(strcmp(fileformat, ".mpg") != 0) sprintf(tosnd, "NOTVF%s", namefile);
						else sprintf(tosnd, "valid%s", namefile);
						//if(fpr != NULL) fclose(fpr);
						//fclose(fp);
						//free(fp);
						int n = send(sd, tosnd, strlen(tosnd), 0);
						if(strncmp(tosnd, "valid", 5) == 0){
							Mat imgServer;
	    						VideoCapture cap(bufbuf);	
	    						int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	   						int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
							char fsize[BUFF_SIZE];
							sprintf(fsize, "%d!%d", width, height);
							n = send(sd, fsize, strlen(fsize), 0);
									
							imgServer = Mat::zeros(height, width, CV_8UC3);    
	    						if(!imgServer.isContinuous()){
		 						imgServer = imgServer.clone();
	    						}
							char comm[5];
							int f;
							while(1){
	       								cap >> imgServer;
									int imgSize = imgServer.total() * imgServer.elemSize();
									char Size[100];
									memset(&Size, 0, sizeof(Size));
									sprintf(Size, "%d", imgSize);
									f = send(sd, Size, strlen(Size), 0);
									int total = 0;
									uchar buffer[imgSize];
									memcpy(buffer,imgServer.data, imgSize);
		 							while(total < imgSize){					
											f = send(sd, buffer+total, imgSize, 0);
											total += f;
									}
									
									memset(&comm, 0, sizeof(comm));
									f = recv(sd, comm, 4, 0);
									if(strncmp(comm, "EXIT", 4) == 0){									
										break;
									}
									
								}
								
								cap.release();
									
								//fclose(fp);		
						}
				//fclose(fp);	
					
}

using namespace std;
int main(int argc, char** argv){
	signal(SIGPIPE, SIG_IGN);
    	int localSocket, remoteSocket;
    	int port = atoi(argv[1]);
	int opt = 1;
	int max_sd, sd; 
	fd_set readfds;
	int client_socket[MAX_CLIENT]; //maximum client allowed: 50
	for(int i = 0; i<MAX_CLIENT; i++)
		client_socket[i] = 0;                             
	struct  sockaddr_in localAddr,remoteAddr;  
        int addrLen = sizeof(struct sockaddr_in);  
        localSocket = socket(AF_INET , SOCK_STREAM , 0);
   	if (localSocket == -1){
        	fprintf(stderr, "socket() call failed!!");
        	return 0;
    	}
	if (setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
		fprintf(stderr, "setsockopt error!");
	
    	localAddr.sin_family = AF_INET;
    	localAddr.sin_addr.s_addr = INADDR_ANY;
    	localAddr.sin_port = htons(port);
	int check = mkdir("server_file", 0777); 
	//if(check < 0) 
	//	printf("Unable to create a new folder\n");
	//char Message[BUFF_SIZE];

   	if(bind(localSocket,(struct sockaddr *)&localAddr , sizeof(localAddr)) < 0) {
            	fprintf(stderr, "Can't bind() socket");
            	return 0;
        }
        listen(localSocket , 3);
    	int x = 0;
	int activity;
    	while(1){
		if(x == 0)
        		std::cout <<  "Waiting for connections...\n"
        	        	<<  "Server Port:" << port << std::endl;
		x = 1;
		FD_ZERO(&readfds);
		FD_SET(localSocket, &readfds);	
		max_sd = localSocket;
		for(int i = 0; i<MAX_CLIENT; i++){
			sd = client_socket[i];
			if(sd > 0)
				FD_SET(sd, &readfds);
			if(sd > max_sd) max_sd = sd;
		}
		activity = select(max_sd+1, &readfds, NULL, NULL, NULL);
		if(activity < 0 && errno!=EINTR)
			fprintf(stderr, "select error.\n"); 	
		if(FD_ISSET(localSocket, &readfds)){
			//desp:
        		remoteSocket = accept(localSocket, (struct sockaddr *)&localAddr, (socklen_t*)&addrLen);  
        		if (remoteSocket < 0) {
            			printf("accept failed!");
            		return 0;
        		}
			printf("New connection, socket fd is %d, ip is %s, port: %d\n", remoteSocket, inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));
			
		//if(x == 0) std::cout << "Connection accepted" << std::endl;
			for(int i = 0; i<MAX_CLIENT; i++){
				if(client_socket[i] == 0){
					client_socket[i] = remoteSocket;
					break;
				}
			}
		}
		desp:
		for(int i = 0; i<MAX_CLIENT; i++){
			sd = client_socket[i];
			if(FD_ISSET(sd, &readfds)){
				char Message[BUFF_SIZE];
				bzero(Message, sizeof(char)*BUFF_SIZE);
				int valread = recv(sd, Message, sizeof(Message), 0);
				int len = strlen(Message);
				Message[len] = '\0';
				//fprintf(stderr, "%s", Message);
				if(valread == 0){
					getpeername(sd, (struct sockaddr*)&remoteAddr, (socklen_t*)&addrLen);
					fprintf(stderr, "Host disconnected: ip: %s, port: %d\n",inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));
					close(sd);
					client_socket[i] = 0;   
				}
				else{
					//Message[valread] = '\0';
					char buffer[BUFF_SIZE];
					bzero(buffer, sizeof(char)*BUFF_SIZE);

					if(strncmp(Message, "ATask", 5) == 0){
						ls_function("server_file/", remoteSocket);
					}
					else if(strncmp(Message, "put", 3) == 0){
						if(strcmp(Message+3, "NOFILE") != 0){
							char temp[BUFF_SIZE];
							bzero(temp, sizeof(char)*BUFF_SIZE);
							strcpy(temp, Message+3);
							long long int sz = atoll(temp);
							fprintf(stderr, "Number of bytes to be sent: %lld\n", sz);
							char namefile[BUFF_SIZE];
							bzero(namefile, sizeof(char)*BUFF_SIZE);
							int recname = recv(sd, namefile, sizeof(namefile), 0);
							long long int remainingData = 0;
							char ubuf[BUFF_SIZE] = "server_file/";
							strcat(ubuf, namefile);
							FILE *fp = fopen(ubuf, "wb"); 
							remainingData = sz;
							ssize_t len;
							char msgBuffer[BUFF_SIZE];
							memset(&msgBuffer, 0, sizeof(msgBuffer));
							ssize_t total = 0;
							while(remainingData != 0)
							{
								if(remainingData < 1024)
								{
									len = recv(sd, msgBuffer, remainingData, 0);
									fwrite(msgBuffer, sizeof(char), len, fp);
									remainingData -= len;
									total += len;
									break;						
								}
								else{
									len = recv(sd, msgBuffer, 1024, 0);
									fwrite(msgBuffer, sizeof(char), len, fp);
									remainingData -= len, total += len;
								}
							}
							fclose(fp);
							int bizarre = recv(sd, msgBuffer, 1024, 0); //receive any weird lingering packet 
							memset(&msgBuffer, 0, sizeof(msgBuffer));
							fprintf(stderr, "Received %ld bytes in total.\n", total);
						}
					}
					else if(strncmp(Message, "get", 3) == 0){
						char namefile[BUFF_SIZE];
						memset(&namefile, 0, sizeof(namefile));
						strcpy(namefile, Message+3);
						char bufbuf[BUFF_SIZE] = "server_file/";
						strcat(bufbuf, namefile);
						//fprintf(stderr, "%s", namefile);
						char name[1024];
						memset(&name, 0, sizeof(name));
						FILE *fpr = fopen(bufbuf, "rb");
						if(fpr == NULL) {strcpy(name, "error!");
							strcat(name, namefile); }
						else strcpy(name, namefile);
						int ne = send(sd, name, sizeof(name), 0);
						if(strcmp(name, namefile) == 0){
							long long int sz = findsize(bufbuf);
							char temp[BUFF_SIZE];
							memset(&temp, 0, sizeof(temp));
							sprintf(temp, "%lld", sz);	
							int ssize = send(sd, temp, sizeof(temp), 0);
							char byteArray[1024];
							memset(&byteArray, 0, sizeof(byteArray));
							int bufread = 0;
							int rembytes = sz;
							//int b;
							while(rembytes != 0){
								if(rembytes < 1024){
								bufread = fread(byteArray, 1, rembytes, fpr);
								rembytes -= bufread;
								ssize = send(sd, byteArray, 1024, 0);
								break;
								}
								else
								{
								bufread = fread(byteArray,1, 1024, fpr);
								rembytes -= bufread;
								ssize = send(sd, byteArray, 1024, 0);						
								}
							}
							fclose(fpr);
							memset(&name, 0, sizeof(name));
							memset(&namefile, 0, sizeof(namefile));
							memset(&name, 0, sizeof(name));
							memset(&byteArray, 0, sizeof(byteArray));
							memset(&temp, 0, sizeof(temp));
						}
					}	
					else if(strncmp(Message, "play", 4) == 0){
						
						shad(sd, Message);
					
					}	
							
				}

			}
		}

    	}
	close(remoteSocket);	
	return 0;
}


