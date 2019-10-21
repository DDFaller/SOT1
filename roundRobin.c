#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	while(1){
	    printf("/Eu sou a saida do round robin/\n");
        sleep(1);
	}
	return 0;
}

void signalHandler(int sig){

	printf("Executando roundRobin.c");
}
