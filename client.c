#include <arpa/inet.h> // inet_addr() 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> // bzero() 
#include <sys/socket.h> 
#include <unistd.h> // read(), write(), close() 
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




void gotoxy(int x, int y) {
    printf("%c[%d;%df", 0x1B, y, x);
}

void set_terminal_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

// Funkcja do przywrócenia domyślnego trybu terminala
void reset_terminal_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}


char wall = '.';

void print_board() {
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
	printf("%d\n", clinet_id);
		

}
char znak = '0';

char xyz[MAX];
void send_bat(int sockfd) {
	bzero(xyz, sizeof(xyz)); 
	xyz[0] = clinet_id;
	xyz[1] = my_bat_position;
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
		znak = '\xff';
	}
}

void *func(void* sockfd_) 
{ 
	int* tmp = (int*)sockfd_;
	int sockfd = *tmp;
	char buff[MAX]; 
	int ball_x, ball_y, client_x, opponent_x;
	int server_player_one, server_player_two, score_one, score_two, print_oponent;
	char* title1 = "  _____   ____  _   _  _____    _____          __  __ ______\0";
	char* title2 = " |  __ \\ / __ \\| \\ | |/ ____|  / ____|   /\\   |  \\/  |  ____|\0";
	char* title3 = " | |__) | |  | |  \\| | |  __  | |  __   /  \\  | \\  / | |__   \0";
	char* title4 = " |  ___/| |  | | . ` | | |_ | | | |_ | / /\\ \\ | |\\/| |  __|  \0";
	char* title5 = " | |    | |__| | |\\  | |__| | | |__| |/ ____ \\| |  | | |____\0";
	char* title6 = " |_|     \\____/|_| \\_|\\_____|  \\_____/_/    \\_\\_|  |_|______|\0";
	char* title[7] = {title1, title2, title3, title4, title5, title6};

	if (clinet_id == 1) {
		client_x = 2;
		opponent_x = 2 + WIDTH -1;
	} else {
		client_x = 2 + WIDTH - 1;
		opponent_x = 2;
	}

	for (;;) { 
		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 

		ball_x = buff[0];
		ball_y = buff[1];
		server_player_one = buff[2];
		server_player_two = buff[3];
		score_one = buff[4];
		score_two = buff[5];
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

		gotoxy(0, HEIGTH + 5);
		for (int i = 0; i < 6; i++) {
        	printf("%s\n", title[i]);
    	} 


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
	servaddr.sin_addr.s_addr = inet_addr("192.168.56.2"); 
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

	void* ptr_sockfd = &sockfd;
	pthread_t w, a;
	pthread_create(&w, NULL, getCharacter, NULL);
	pthread_create(&a, NULL, func, ptr_sockfd);
	pthread_join(w, NULL);
	pthread_join(a, NULL);




 
	// close the socket 
	close(sockfd);
	
	
	reset_terminal_mode();
	return 0;
} 