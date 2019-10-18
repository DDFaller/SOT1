#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include "Interpretador.h"
#include <unistd.h>
#include <time.h>
#define IO 1

int main(void) {
    time_t lastTime;
    time_t seconds;
	FILE * arq;
	int wait;
	arq = fopen("desc.txt", "r");
    lastTime = time(NULL);    
    Interpreter(arq);
    while(1){
        if(time(NULL) - lastTime >= IO * 60){        
            Interpreter(arq);
			lastTime = time(NULL);
        }
        
    }	
    
	fclose(arq);
	scanf("%d", &wait);

}
