#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	int input = 0;
	
	while( 1){
		printf("/Eu sou a saida do priority/\n");
		usleep(40000);
	}
	
	
	return 0;
}

void signalHandler(int sig){

	printf("Executando priority.c");
}
