#include <arpa/inet.h> // inet_addr() 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> // bzero() 
#include <sys/socket.h> 
#include <unistd.h> // read(), write(), close() 

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

void gotoxy(int x, int y) {
    printf("%c[%d;%df", 0x1B, y, x);
}

void print_board(int x, int y) {
	//system("clear");
	

}


void func(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 
	int x, y, cntr[4] = {-1, -1, -1, -1};
	for (;;) { 
		bzero(buff, sizeof(buff)); 
		/*
		printf("Enter the string : "); 
		n = 0; 
		while ((buff[n++] = getchar()) != '\n') 
			; 
		write(sockfd, buff, sizeof(buff)); 
		bzero(buff, sizeof(buff));
		*/ 
		read(sockfd, buff, sizeof(buff)); 
		if(cntr[0] != buff[0] || cntr[1] != buff[1] || cntr[2] != buff[3] || cntr[3] != buff[4] ) {
			cntr[0] = buff[0];
			cntr[1] = buff[1];
			cntr[2] = buff[3];
			cntr[3] = buff[4];
			system("clear");
		}
		x = (buff[0]) * 10;
		x += buff[1];

		y = (buff[3]) * 10;
		y += buff[4];
			
		gotoxy(x,y);
		printf("o");
		fflush(stdout);
			
		if ((strncmp(buff, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		} 
		
		
	} 
} 
 
int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 
 
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 
 
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("192.168.21.2"); 
	servaddr.sin_port = htons(PORT); 
 
	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) 
		!= 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else 
		printf("connected to the server..\n"); 
 
	// function for chat 
	func(sockfd); 
 
	// close the socket 
	close(sockfd);
	
	
	
	return 0;
} 