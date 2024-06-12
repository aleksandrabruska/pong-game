/*****************************
*
* PONG GAME
* Distributed Processing project
* Part 3: Implementation
*
* Aleksandra Bruska, 185454
* Jan Walczak, 193440
* Igor Jozefowicz, 193257
* Gdansk University of Technology
*
******************************/


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
#include <math.h>
#include <semaphore.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
#define WIDTH  60
#define HEIGHT 15
#define PALETTE_LENGTH


sem_t sem;

enum {LEFT, RIGHT};
double ball_x = -1;
double ball_y = 0;
double move_vect[] = {1.0, -0.75};
double prev_start_x = 1;

double player1_pos[] = {0,0};
double player2_pos[] = {WIDTH,0};
int points[] = {0,0};
int points_needed = 10;
int power_mode = 0;
int power_mode_time = 40;

void start_pos(){
	move_vect[0] = -prev_start_x;
	prev_start_x = -prev_start_x;
	move_vect[1] = -0.75;
	ball_x = WIDTH/2;
	ball_y = HEIGHT/2; 
}
void my_sleep(){
	time_t time = clock();
	while(clock() - time < 250000){
		//printf("%d\n", clock() - time);
	}
}

int check_for_hit(){
	if(ball_x <= player1_pos[0] && ball_y >= (int)(player1_pos[1])
			&& ball_y <= (int)(player1_pos[1] +3)){
		if(move_vect[0] < 0){
			move_vect[0] = - move_vect[0];
			//points[0]++;
			return 1;
		}
		
	}
	else if(ball_x >= player2_pos[0]-2 && ball_y >= (int)(player2_pos[1])
			&&ball_y <= (int)(player2_pos[1] +3)){
		if(move_vect[0] > 0){
			move_vect[0] = - move_vect[0];
			//points[1]++;
			return 1;
		}
		
	}
	return 0;
}

char move_ball(){
	if(check_for_hit() == 0){

		if(ball_y <=0 && move_vect[1] <0){
			move_vect[1] = -move_vect[1];
		}
		else if(ball_y >= HEIGHT-1 && move_vect[1] > 0){
			move_vect[1] = - move_vect[1];
		}
		//f(ball_x <= 0 && move_vect[0] < 0  || ball_x >= WIDTH && move_vect[1] > 0){
		if(ball_x <= 0 && move_vect[0] <= 0 || ball_x >= WIDTH-1 && move_vect[0] >= 0){
			//move_vect[0] = - move_vect[0];

			if(move_vect[0] > 0){
				points[0]++;
			}
			if(move_vect[0] < 0){
				points[1]++;
			}
			
			start_pos();
			if(points[0] >= points_needed || points[1] >= points_needed)
				return 1;
		}
	}
	if(power_mode){
		ball_x += 2 * move_vect[0];	
		ball_y += 2 * move_vect[1];
	}
	else{
		ball_x += move_vect[0];	
		ball_y += move_vect[1];
	}
	return 0;
	//ball_x += 1;
	
}

void read_paddle(int connfd_){
	char buff[MAX]; 

	read(connfd_, buff, sizeof(buff)); 
	
	if(buff[0] == 1){
		int z = buff[1];
		player1_pos[1] = z;
		printf("Player 1 pos %lf %lf\n", player1_pos[0], player1_pos[1]);
	}
	else{
		int z = buff[1];
		player2_pos[1] = z;
		printf("Player 2 pos %lf %lf\n", player2_pos[0], player2_pos[1]);
	}
	if(buff[2] == 1){
		printf("Setting powermode on");
		sem_wait(&sem);
		if(power_mode == 0){
			power_mode = 1;
			power_mode_time = 40;
		}
		sem_post(&sem);
	}
	bzero(buff, MAX); 
}
int end_of_game;


void send_info(int connfd_){
	int n = 0;
	
	char buff[MAX]; 
	int ball_x_i = ball_x;
	int ball_y_i = ball_y;
	buff[n++] = ball_x_i;
	buff[n++] = ball_y_i;;
	buff[n++] = player1_pos[1];
	buff[n++] = player2_pos[1];
	buff[n++] = points[0];
	buff[n++] = points[1];
	buff[n++] = end_of_game;
	
		
	
	printf("Power mode %d", power_mode_time);
	printf("SCORE p_one=%d  p_two=%d,	end = %d\n", buff[4], buff[5], buff[6]);
 
	
	write(connfd_, buff, sizeof(buff)); 
	bzero(buff, MAX); 

}


void* client_thread_func(void* connfd_) 
{ 
	//my_sleep();
	int prev_x = 0;
	int* connfd_p = (int*) connfd_;
	int connfd = *connfd_p;
	printf("in func");
	char buff[MAX]; 
	int n; 

	for (;;) { 
		if(prev_x != ball_x){
		//sleep(1);
			prev_x= ball_x;
			bzero(buff, MAX); 
	
			
			if (strncmp("exit", buff, 4) == 0) { 
				printf("Server Exit...\n"); 
				break; 
			} 
			send_info(connfd);
			read_paddle(connfd);
			
		}
		
		//wait(100);
	} 
	
} 

void* server_thread(void* arg){
	for (;;) { 
		if(power_mode == 1 && power_mode_time > 0){
			power_mode_time -= 1;
		}
		else if(power_mode_time == 0 && power_mode != 0){
			power_mode = 0;
			power_mode_time = 40;
		}


		my_sleep();
		if(move_ball() == 1) {
			if(points[0] == points_needed)
				end_of_game = 1;
			else if(points[1] == points_needed)
				end_of_game = 2;
			break;		
		}
		
	
	}
}

int main() 
{ 
	sem_init(&sem, 0, 1);
	end_of_game = 0;
	
	int sockfd, connfd, connfd2, len; 
	struct sockaddr_in servaddr, cli; 
 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Socket successfully created..\n"); 

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    	herror("setsockopt(SO_REUSEADDR) failed");
	
	
	bzero(&servaddr, sizeof(servaddr)); 


 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		printf("Error code: %d\n", errno);
		exit(0); 
	} 
	else 
		printf("Socket successfully binded..\n"); 
 

	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Server listening..\n"); 
	len = sizeof(cli); 
 

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
 

	
	pthread_t w;
	pthread_t w2;
	pthread_t server_th;


	void* connfd_p = &connfd;
	void* connfd_p2 = &connfd2;
	pthread_create (&w , NULL , client_thread_func , connfd_p ) ;
	pthread_create (&w2 , NULL , client_thread_func , connfd_p2 ) ;
	pthread_create (&server_th , NULL , server_thread, NULL) ;

	pthread_join (w , NULL ) ;
	pthread_join (w2 , NULL ) ;
	pthread_join (server_th, NULL ) ;	
	

	close(sockfd); 
} 
