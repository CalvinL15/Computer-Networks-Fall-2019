#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/inet.h>

typedef struct {
	int length;
	int seqNumber;
	int ackNumber;
	int fin;
	int syn;
	int ack;
} header;

typedef struct{
	header head;
	char data[1000];
} segment;

int max(int a, int b){
	if(a > b)
		return a;
	return b;
}

int create_socket(int port, char *ip){
	int socket_fd;
	struct sockaddr_in server_addr;
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	return socket_fd;
}
void set_addr(struct sockaddr_in* addr, char* ip, int port){
	memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);
    memset(addr->sin_zero, '\0', sizeof(addr->sin_zero));  
}

int send_packet(int fd, segment* p, struct sockaddr_in* addr){
	int nbytes;
	int length = sizeof(*addr);
	nbytes = sendto(fd, p, sizeof(*p), 0, (struct sockaddr*)addr, *(socklen_t*)&length);
	return nbytes;
}

int recv_packet(int fd, segment* p, struct sockaddr_in* addr){
	int nbytes;
	int length = sizeof(*addr);
	nbytes = recvfrom(fd, p, sizeof(*p), 0, (struct sockaddr*)addr, (socklen_t*)&length);
	return nbytes;
}
