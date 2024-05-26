#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include<errno.h>
#include<unistd.h>
#include <pthread.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
 
int ball_x = 0;
int ball_y = 0;


// Function designed for chat between client and server. 

void my_sleep(){
	time_t time = clock();
	while(clock() - time < 1000000){
		//printf("%d\n", clock() - time);
	}
}
void move_ball(){
	ball_x += 1;
	ball_y += 1;
}
void* func(void* connfd_) 
{ 
	//my_sleep();
	int prev_x = 0;
	int* connfd_p = (int*) connfd_;
	int connfd = *connfd_p;
	printf("in func");
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	for (;;) { 
		if(prev_x != ball_x){
		//sleep(1);
			prev_x= ball_x;
			bzero(buff, MAX); 
	
			// read the message from client and copy it in buffer 
			//read(connfd, buff, sizeof(buff)); 
			// print buffer which contains the client contents 
			//printf("From client: %s\t To client : ", buff); 
			//bzero(buff, MAX); 
			n = 0; 
			// copy server message in the buffer 
			//while ((buff[n++] = getchar()) != '\n') 
			//	;
			//move_ball();
			buff[n++] = ball_x / 10;
			buff[n++] = ball_x % 10;
			buff[n++] = ' ';
			buff[n++] = ball_y/ 10;
			buff[n++] = ball_y % 10;
			buff[n++] = '\0';
			printf("Sending %d %d %d %d\n", buff[0], buff[1], buff[3], buff[4]);
			// and send that buffer to client 
			write(connfd, buff, sizeof(buff)); 
	
			// if msg contains "Exit" then server exit and chat ended. 
			if (strncmp("exit", buff, 4) == 0) { 
				printf("Server Exit...\n"); 
				break; 
			} 
		}
		//wait(100);
	} 
	
} 
 

void* server_thread(void* arg){
	for (;;) { 
		my_sleep();
		move_ball();
	}
}
// Driver function 
int main() 
{ 
	
	
	int sockfd, connfd, connfd2, len; 
	struct sockaddr_in servaddr, cli; 
 
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Socket successfully created..\n"); 

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    	error("setsockopt(SO_REUSEADDR) failed");
	
	
	bzero(&servaddr, sizeof(servaddr)); 


 
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 
 
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		printf("Error code: %d\n", errno);
		exit(0); 
	} 
	else 
		printf("Socket successfully binded..\n"); 
 
	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Server listening..\n"); 
	len = sizeof(cli); 
 
	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server accept failed...\n"); 
		exit(0); 
	} 
	else 
		printf("server accept the client...\n"); 

	connfd2 = accept(sockfd, (SA*)&cli, &len); 
	if (connfd2 < 0) { 
		printf("server accept failed...\n"); 
		exit(0); 
	} 
	else 
		printf("server accept the client number 2...\n"); 
 
	// Function for chatting between client and server 
	
	pthread_t w;
	pthread_t w2;
	pthread_t server_th;


	void* connfd_p = &connfd;
	void* connfd_p2 = &connfd2;
	pthread_create (&w , NULL , func , connfd_p ) ;
	pthread_create (&w2 , NULL , func , connfd_p2 ) ;
	pthread_create (&server_th , NULL , server_thread, NULL) ;

	pthread_join (w , NULL ) ;
	pthread_join (w2 , NULL ) ;
	pthread_join (server_th, NULL ) ;	
	
	//printf (" Hello world \ n ") ;
	//func(connfd); 
 
	// After chatting close the socket 
	close(sockfd); 
} 