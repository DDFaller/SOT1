#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	int input = 0;
		
	signal(SIGUSR1,signalHandler);
	printf("Vivo \n");
	
	while( input < 1000){
		printf("INPUT %d:P\t",input);
		input += 1;
		sleep(1);
	}
	
	
	return 0;
}

void signalHandler(int sig){

	printf("Executando priority.c");
}
