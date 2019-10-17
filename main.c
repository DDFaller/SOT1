#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include "Interpretador.h"
#include <unistd.h>

#define IO 1

int main(void) {
	FILE * arq;
	int wait;
	arq = fopen("desc.txt", "r");
    while(1){
        Interpreter(arq);
        sleep(IO);
    }	
    
	fclose(arq);
	scanf("%d", &wait);

}
