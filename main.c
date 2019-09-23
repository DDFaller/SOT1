#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include "Interpretador.h"
#define macroteste(h)  h*h

int main(void) {
	FILE * arq;
	int wait;
	arq = fopen("desc.txt", "r");
	Interpreter(arq);
	fclose(arq);
	scanf("%d", &wait);

}