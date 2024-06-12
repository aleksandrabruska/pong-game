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

#include <arpa/inet.h> 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <pthread.h>

#include <unistd.h>
#include <termios.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

#define WIDTH 60
#define HEIGTH 15
#define BAT_H 3

int clinet_id;
int my_bat_position = HEIGTH/2;
int opponent_bat_position = HEIGTH/2;
int set_power_mode = 0;
int time_to_next_power_mode = 100;



void gotoxy(int x, int y) {
    printf("%c[%d;%df", 0x1B, y, x);
}

void set_terminal_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}


void reset_terminal_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}


char wall = '.';
int score_one, score_two, end, ball_x, ball_y;

char* title1 = "  _____   ____  _   _  _____    _____          __  __ ______\0";
char* title2 = " |  __ \\ / __ \\| \\ | |/ ____|  / ____|   /\\   |  \\/  |  ____|\0";
char* title3 = " | |__) | |  | |  \\| | |  __  | |  __   /  \\  | \\  / | |__   \0";
char* title4 = " |  ___/| |  | | . ` | | |_ | | | |_ | / /\\ \\ | |\\/| |  __|  \0";
char* title5 = " | |    | |__| | |\\  | |__| | | |__| |/ ____ \\| |  | | |____\0";
char* title6 = " |_|     \\____/|_| \\_|\\_____|  \\_____/_/    \\_\\_|  |_|______|\0";
void print_board() {
	char* title[7] = {title1, title2, title3, title4, title5, title6};

	system("clear");

    for (int i = 0; i <= HEIGTH + 1; i++) {
		if (!i || i == HEIGTH + 1)  {
            for (int j = 0; j < WIDTH + 2; j++)
                printf("#");
            printf("\n");

		} else {
            for (int j = 0; j < WIDTH; j++) {
			    if(!j)
				    printf("%c", wall);
                printf(" ");
		}
		printf("%c\n", wall);
        }
    }

	printf("You are the client number: %d\n", clinet_id);
	
	printf("Your points: ");
	if(clinet_id == 1) {
		printf("%d\nYour opponent's points: %d\n", score_one, score_two);
	} else {
		printf("%d\nYour opponent's points: %d\n", score_two, score_one);
	}


	

	for (int i = 0; i < 6; i++)
        printf("%s\n", title[i]);

	char hashappened = 0;
	if(set_power_mode == 1){
		hashappened = 1;
	}


	if(end) {
		gotoxy(WIDTH/2 + 2 - 22, HEIGTH/2);
		printf("END OF GAME, PLAYER -- %d -- WON THE GAME!!", end);
	}
	
	

}
char znak = '0';

char xyz[MAX];

void send_bat(int sockfd) {
	bzero(xyz, sizeof(xyz)); 
	xyz[0] = clinet_id;
	xyz[1] = my_bat_position;
	xyz[2] = set_power_mode;
	set_power_mode = 0;
	write(sockfd, xyz, sizeof(xyz));
}

void* getCharacter(void* a) {
	znak = '\xff';
	for(;;) {
		znak = getchar();
		if(znak == 'w') {
			if(my_bat_position > 0)
				my_bat_position--;
		} 
		else if (znak == 's'){
			if(my_bat_position < HEIGTH - BAT_H)
				my_bat_position++;
		}
		else if (znak == 'p'){
			if(time_to_next_power_mode == 0){
				set_power_mode = 1;
			}
		}
		znak = '\xff';
	}
}

void *func(void* sockfd_) 
{ 
	int* tmp = (int*)sockfd_;
	int sockfd = *tmp;
	char buff[MAX]; 
	int client_x, opponent_x;
	int server_player_one, server_player_two, print_oponent;
	

	if (clinet_id == 1) {
		client_x = 2;
		opponent_x = 2 + WIDTH -1;
	} else {
		client_x = 2 + WIDTH - 1;
		opponent_x = 2;
	}

	for (;;) { 
		if(time_to_next_power_mode > 0){
			time_to_next_power_mode -= 1;
		}

		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 

		ball_x = buff[0];
		ball_y = buff[1];
		server_player_one = buff[2];
		server_player_two = buff[3];
		score_one = buff[4];
		score_two = buff[5];
		end = buff[6];
		if (clinet_id == 1) {
			print_oponent = server_player_two;
		}  else {
			print_oponent = server_player_one;
		}

		print_board();
		gotoxy(ball_x + 3, ball_y + 2);
		printf("o");
		
   		
		for (int i = 0; i < BAT_H; i++) {
			gotoxy(client_x, my_bat_position + 2 + i);
			printf("|");
		}

		for (int i = 0; i < BAT_H; i++) {
			gotoxy(opponent_x, print_oponent + 2 + i);
			printf("|");
		}
		
		printf("\e[?25l");

		


		fflush(stdout);
		
		send_bat(sockfd);

	} 
} 
 
int main(int argc, char *argv[]) 
{ 
	set_terminal_mode();
	clinet_id = argv[1][0] - '0';
	
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 
 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else 
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("192.168.56.2"); 
	servaddr.sin_port = htons(PORT); 
 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) 
		!= 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else 
		printf("connected to the server..\n"); 
 

	void* ptr_sockfd = &sockfd;
	pthread_t w, a;
	pthread_create(&w, NULL, getCharacter, NULL);
	pthread_create(&a, NULL, func, ptr_sockfd);
	pthread_join(w, NULL);
	pthread_join(a, NULL);


	close(sockfd);
	
	
	reset_terminal_mode();
	return 0;
} 
