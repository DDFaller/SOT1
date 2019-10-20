#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>

void signalHandler(int sig);

int main(int argc, char * argv[]){
	int input = 0;
	float time = 0.5;	
	printf("Vivo \n");
	
    if(time > input){
        printf("Teste de > \n");    
    }
	if((time - (int)time) != 0){
        printf("Teste segundos");   
    }
    printf("%f",time - input);
	
	
	return 0;
}

void signalHandler(int sig){

	printf("Executando priority.c");
}
