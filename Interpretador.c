#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Interpretador.h"
#include "Escalonador.h"
#define TRUE 1
#define FALSE 0
#define print printf
#define WHITESPACE '?'
#define ValidCharacters(c) if (c != ' ' && c != '\n' && c != EOF && c != '=' && c!= '*')


char ReadIgnoringCharacter(FILE *f, char ignoredCharacter) {
	char c;
	c = fgetc(f);
	while (TRUE) {
		switch (ignoredCharacter)
		{
		case(WHITESPACE):
			if (c != ' ' && c != '\n') {
				return c;
			}
		default:
			if (c != ignoredCharacter) {
				return c;
			}
		}
		c = fgetc(f);
	}
	return c;
}

char ReadWord(FILE * f, char ** word) {
	char c;
	int idx = 0;
	char * nameHolder = (char*)malloc(sizeof(char) * 20);
	char * name = (char*)malloc(sizeof(char) * 20);

	c = ReadIgnoringCharacter(f, WHITESPACE);
	nameHolder[idx++] = c;
	while (TRUE) {
		c = fgetc(f);
		ValidCharacters(c) {
			nameHolder[idx++] = c;
		}
		else {
			break;
		}
	}
	*(nameHolder + idx++) = '\0';
	strcpy(name, idx, nameHolder);
	free(nameHolder);
	*word = name;
	return c;
}

Escalonadores ReadLine(FILE * f, char ** fileName, char ** opr,int * firstDuration, int * secondDuration) {
	char * t_firstDuration;
	char * t_secondDuration;
	char * t_operator;
	char * t_fileName;
	char * cmp = (char*)malloc(sizeof(char) * 20);
	char lastChar;
	
	
	fread(cmp, sizeof(char), 3, f);
	if (strcmp(cmp, "Run") || strcmp(cmp, "run")) {
		lastChar = ReadWord(f, &t_fileName);
		*fileName = t_fileName;

		if (lastChar == "\n")
			return roundRobin;
		//else " "
		
		lastChar = ReadWord(f, &t_operator);
		*opr = t_operator;
		if (lastChar == '=') {
			lastChar = ReadWord(f, &t_firstDuration);
		}
		else {
			printf("Não foi informado a duração 1 para execução do %s \n", t_fileName);
			return Unknown;
		}
		*firstDuration = atoi(t_firstDuration);
		if (lastChar == '\n')
			return prioridade;
		//else " "

		lastChar = ReadWord(f, &t_operator);
		if (lastChar == '=') {
			lastChar = ReadWord(f, &t_secondDuration);
		}
		else {
			printf("Não foi informado a duração 2 para execução do %s \n", t_fileName);
			return Unknown;
		}
		*secondDuration = atoi(t_secondDuration);
		return RealTime;
	}	
}
void Interpreter(FILE * f) {
	char * palavra;
	char * fileName;
	char * opr;
	int firstDuration = -1;
	int secondDuration = -1;
	Escalonadores type;
	type = ReadLine(f,&fileName,&opr,&firstDuration,&secondDuration);
	switch (type)
	{
		case(prioridade):
            printf("Adicionando processo  na fila de Prioridade \n");		
            priority(fileName, firstDuration);
			break;
		case(roundRobin):
            printf("Adicionando processo  na fila de Round Robin \n");
			roundrobin(fileName);
			break;
		case(RealTime):
            printf("Adicionando processo na fila de Real Time \n");
			realTime(fileName,firstDuration,secondDuration);
			break;
		default:
			break;
	}
    printf("TIPO\tNAME\tOPR\tInicio\tDuracao");
	printf("%d\t%s\t%s\t%d\t%d\t\n",type, fileName, opr, firstDuration, secondDuration);
	printf("-------------------------------------------");
}


