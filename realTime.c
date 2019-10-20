#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	int input = 0;
		
	printf("Vivo \n");
	
	while( 1){
		                  printf("RealTImejfkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk \n");
		sleep(1);
	}
	
	
	return 0;
}

void signalHandler(int sig){

	printf("Executando priority.c");
}
