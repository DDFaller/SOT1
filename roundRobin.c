#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	time_t  lastTime= time(NULL);
	while(1){
		if(time(NULL) - lastTime >= 1){
			printf(" R ");
			lastTime = time(NULL);
		}
	}
	
	
	return 0;
}

void signalHandler(int sig){

	printf("Executando roundRobin.c");
}
