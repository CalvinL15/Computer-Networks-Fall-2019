#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include "opencv2/opencv.hpp"
#include <signal.h>


#define _OPEN_SYS_ITOA_EXT

#define BUFF_SIZE 1024
#define MAX_LINE 2048
#define MAXIMUM 1000000
using namespace cv;
typedef long long int LLI;

LLI findsize(char *filename){
	FILE *fp = fopen(filename, "r");
	fseek(fp, 0LL, SEEK_END);
	LLI res = ftell(fp);
	fclose(fp);
	return res;	
}

void shad(int localSocket, char command[BUFF_SIZE]){
			char msgbuf[BUFF_SIZE];
			memset(&msgbuf, 0, sizeof(msgbuf));
			strcpy(msgbuf, command);
			int n = send(localSocket, msgbuf, BUFF_SIZE, 0);
			char check[BUFF_SIZE];
			memset(&check, 0, sizeof(check));
			int m = recv(localSocket, check, BUFF_SIZE, 0);
			
			char toprint[BUFF_SIZE];
			memset(&toprint, 0, sizeof(toprint));
			if(strncmp(check, "DNE", 3) == 0){
				strcpy(toprint, check+3);
				fprintf(stderr, "The '%s' does not exist.\n", toprint);
			}
			else if(strncmp(check, "NOTVF", 5) == 0){
				strcpy(toprint, check+5);
				fprintf(stderr, "The '%s' is not a mpg file.\n", toprint);
			}
			else{
				Mat imgclient;
				char fsize[100], width[100], height[100];
				memset(&width, 0, sizeof(width));
				memset(&height, 0, sizeof(height));
				int k = recv(localSocket, fsize, 100, 0);
				int x = 0;
				while(fsize[x] != '!')
					width[x] = fsize[x], x++;
				width[x] = '\0';
				strcpy(height, fsize+x+1);
				int rW = atoi(width);
				int rH = atoi(height);	
				imgclient = Mat::zeros(rH, rW, CV_8UC3);	
				if(!imgclient.isContinuous()){
         				imgclient = imgclient.clone();
    				}
				char f[5];
				memset(&f, 0, sizeof(f));
				while(1){
					char szsave[BUFF_SIZE];
					//memset(&szsave, 0, sizeof(szsave));
					k = recv(localSocket, szsave, BUFF_SIZE, 0);
					int actsz;
					sscanf(szsave, "%d", &actsz);
					
 					int total = 0;
					int left = actsz;
					uchar bufferr[actsz];
					//memset(&bufferr, 0, sizeof(bufferr));
					while(total < left){
						k = recv(localSocket, bufferr+total, left, 0);
						total += k;
					}
					
					uchar *iptr = imgclient.data;
        				memcpy(iptr,bufferr,actsz);
 					
					imshow("Video", imgclient);
					char c = (char)waitKey(33.3333);
				    	if(c==27){
						sprintf(f, "EXIT");
						k = send(localSocket, f, strlen(f), 0);
						break;
					}
					else{
						sprintf(f, "CONT");
						 k = send(localSocket, f, strlen(f), 0);   
						
					}       
					memset(&f, 0, sizeof(f));     				
				}
				destroyAllWindows();
			}
				

}

using namespace std;
int main(int argc , char *argv[])
{
		int check = mkdir("client_file", 0777); 
		signal(SIGPIPE, SIG_IGN);		
		char *head = strtok(argv[1], ":");
		char ip[35];
		int port;
		int F = 0;
		while(head != NULL){
			if(F == 0){
				strcpy(ip, head);
				F++;
			}
			else{
				port = atoi(head);
			}
			head = strtok(NULL, ":");
		}
    	int localSocket, recved;
    	localSocket = socket(AF_INET , SOCK_STREAM , 0);

    	if (localSocket == -1){
        	fprintf(stderr, "Fail to create a socket.\n");
        	return 0;
    	}

    	struct sockaddr_in info;
    	bzero(&info,sizeof(info));

    	info.sin_family = PF_INET;
    	info.sin_addr.s_addr = inet_addr(ip);
    	info.sin_port = htons(port);
    	int err = connect(localSocket,(struct sockaddr *)&info,sizeof(info));
    	if(err==-1){
        	fprintf(stderr, "Connection error\n");
        	return 0;
    	}
	else fprintf(stderr, "Connection Accepted!\n");
    	char command[BUFF_SIZE] = {};
    	int snd = 0;
    	while(1){
		bzero(command,sizeof(char)*BUFF_SIZE);
		scanf("%[^\n]", command);
		getchar();
		//printf("%s", command);
		if(strncmp(command, "ls", 2) == 0){
			snd = send(localSocket, "ATask", 5, 0);
			char buf[BUFF_SIZE*10];
			memset(&buf, 0, sizeof(buf));
		//	int flag = 1;
		//	int tlen = 0;
			int recdata = recv(localSocket, buf, sizeof(buf), 0);
			/*while(1){
				char buf[BUFF_SIZE];
				memset(&buf, 0, sizeof(buf));
				int recdata = recv(localSocket, buf, sizeof(buf), 0);		
				fprintf(stderr, "%s", buf);
				if(strncmp(buf, "FORBIDDEN", 9) == 0)
					fprintf(stderr, "WTF");
			}*/
			fprintf(stderr, "%s", buf);
		}
		else if(strncmp(command, "put ", 4) == 0 && strlen(command) > 4){
			int flag = 0;			
			char namefile[BUFF_SIZE];
			bzero(namefile, sizeof(char)*BUFF_SIZE);
			strcpy(namefile, command+4);
			//fprintf(stderr, "%s", namefile);
			FILE *fp;
			char ultimatebuf[BUFF_SIZE] = "client_file/";
			strcat(ultimatebuf, namefile);
			fp = fopen(ultimatebuf, "rb");
			char buf_to_send[BUFF_SIZE];
			if(fp == NULL){
				fprintf(stderr, "The '%s' does not exist.\n", namefile);
				sprintf(buf_to_send, "putNOFILE");
			}
			else{
				flag = 1;
				char t[4] = "put";
				sprintf(buf_to_send, "%s%lld", t, findsize(ultimatebuf));
			}
			//fprintf(stderr, "%s", namefile);
			int senddata = send(localSocket, buf_to_send, sizeof(buf_to_send), 0);
			long long int sz = findsize(ultimatebuf);
			if(flag == 1){
				int sendname = send(localSocket, namefile, sizeof(namefile), 0);
				ssize_t total = 0;
				int buffread = 0;
				long long int remainingdata = sz;
				int n;
				char sendArr[BUFF_SIZE];
				memset(&sendArr, 0, sizeof(sendArr));
				while(remainingdata > 0){
					if(remainingdata < 1024){
						buffread = fread(sendArr, 1, remainingdata, fp);
						remainingdata -= buffread, total += buffread;
						n = send(localSocket, sendArr, 1024, 0);
					}
					else{
						buffread = fread(sendArr, 1, 1024, fp);
						remainingdata -= buffread, total += buffread;
						n = send(localSocket, sendArr, 1024, 0);				
					}
				}
				fprintf(stderr, "File sent with total size: %ld\n", total);
				memset(&sendArr, 0, sizeof(sendArr));
				fclose(fp);
			}
		}
		
		else if(strncmp(command, "get ", 4) == 0 && strlen(command) > 4){
			char msgbuf[BUFF_SIZE];
			memset(&msgbuf, 0, sizeof(msgbuf));
			strcpy(msgbuf, command+4);
			char sndcm[BUFF_SIZE] = "get";
			strcat(sndcm, msgbuf);
			int n;
			n = send(localSocket, sndcm, sizeof(sndcm), 0);
			char name[1024];
			memset(&name, 0, sizeof(name));
			n = recv(localSocket, name, 1024, 0);
			if(strncmp(name, "error!", 6) == 0){
				char nme[1024];
				memset(&nme, 0, sizeof(nme));
				strcpy(nme, name+6);
				fprintf(stderr, "The '%s' does not exist\n", nme);
				memset(&nme, 0, sizeof(nme));
			}
			else{
				ssize_t len;
				char savsize[BUFF_SIZE];
				memset(&savsize, 0, sizeof(savsize));
				n = recv(localSocket, savsize, BUFF_SIZE, 0);
				long long int sz = atoll(savsize);
				fprintf(stderr, "Client will 'download' a file with size: %lld\n", sz);
				char bufl[BUFF_SIZE];
				memset(&bufl, 0, sizeof(bufl));
				long long int remdata = sz;
				ssize_t total = 0;
				char path[BUFF_SIZE] = "client_file/";
				char nme[1024];
				memset(&nme, 0, sizeof(nme));
				strcpy(nme, name);
				//fprintf(stderr, "%s", nme);
				strcat(path, nme);
				//fprintf(stderr, "%s", path);
				FILE *fptr = fopen(path, "wb");
				while(remdata != 0){
					if(remdata < 1024)
					{	len = recv(localSocket, bufl, remdata, 0);
						fwrite(bufl, sizeof(char), len, fptr);
						remdata -= len, total += len;
						break;
					}
					else{
						len = recv(localSocket, bufl, 1024, 0);
						fwrite(bufl, sizeof(char), len, fptr);
						remdata -= len, total += len;
					}
				}
				fclose(fptr);
				int n = recv(localSocket, bufl, 1024, 0); //lingering packet
				memset(&bufl, 0, sizeof(bufl)); 
				memset(&nme, 0, sizeof(nme));
				memset(&bufl, 0, sizeof(bufl));
				memset(&path, 0, sizeof(path));
				memset(&savsize, 0, sizeof(savsize));
				memset(&name, 0, sizeof(name));
				memset(&sndcm, 0, sizeof(sndcm));
				fprintf(stderr, "Downloaded %ld bytes in total.\n", total);	
			}

		}
		else if(strncmp(command, "play ", 5) == 0 && strlen(command) > 5){
			shad(localSocket, command);
			

		}
		else if(strcmp(command, "put") == 0 || strcmp(command, "put ") == 0 || strcmp(command, "get") == 0 || strcmp(command,"get ") == 0)
			fprintf(stderr, "Command format error.\n");
		else if(strcmp(command, "play") == 0 || strcmp(command, "play ") == 0)
			fprintf(stderr, "Command format error.\n");
		else fprintf(stderr, "Command not found.\n");			
    }
    printf("close Socket\n");
    close(localSocket);
    return 0;
}


