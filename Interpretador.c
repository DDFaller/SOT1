#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Interpretador.h"
#include "Escalonador.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define print printf
#define WHITESPACE '?'
#define ValidCharacters(c) if (c != ' ' && c != '\n' && c != EOF && c != '=' && c!= '*')
#define FIFO "minhaFifo"
#define OPENMODE O_WRONLY

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
	*(nameHolder + idx) = '\0';
	strcpy(name,nameHolder);
	free(nameHolder);
	*word = name;
    printf("Palavra lida %s ultimo caracter ?%c? \n",name,c);
	return c;
}

Escalonadores ReadLine(FILE * f, char ** fileName, char ** opr,char ** firstDuration, char ** secondDuration) {
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
        

        if(lastChar == '\n'){
            printf("Line break detected \n");
            return roundRobin;        
        }
        //else " "
		
		lastChar = ReadWord(f, &t_operator);

            printf("Operador %s \n",t_operator);           
		*opr = t_operator;
		if (lastChar == '=') {
			lastChar = ReadWord(f, &t_firstDuration);
		}
		else {
			printf("Não foi informado a duração 1 para execução do %s \n", t_fileName);
			return Unknown;
		}
		*firstDuration = t_firstDuration;
        printf("First Duration> %s\n",*firstDuration);
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
        		
        *secondDuration = t_secondDuration;
        printf("Second Duration> %s\n",*secondDuration);		
        return RealTime;
	}	
}
char * Interpreter(FILE * f, int seconds) {
	char * palavra;
	char * fileName;
	char * opr;
    char * command;
    char secondsToStr[2];	
    char whitespace[] = {' ','\0'};
    char * firstDuration;
	char * secondDuration;
    command = (char*)malloc(sizeof(char)*30);
	Escalonadores type;
	type = ReadLine(f,&fileName,&opr,&firstDuration,&secondDuration);
        
    secondsToStr[0] = seconds + 48;
    secondsToStr[1] = '\0';

    strcpy(command,secondsToStr);
	switch (type)
	{
		case(prioridade):
            printf("Adicionando processo  na fila de Prioridade \n");		
            strcat(command," Priority ");
            printf("%s \n",command);            
            strcat(command,fileName);
            strcat(command,whitespace);
            printf("%s \n",command);      
            strcat(command,firstDuration);
            printf("%s \n",command);            
            break;
		case(roundRobin):
            printf("Adicionando processo  na fila de Round Robin \n");
			strcat(command," RoundRobin ");
            printf("%s \n",command);           
            strcat(command,fileName);
            printf("%s \n",command);
            break;
		case(RealTime):
            printf("Adicionando processo na fila de Real Time \n");
			strcat(command," RealTime ");
            printf("%s \n",command);            
            strcat(command,fileName);
            strcat(command,whitespace);
            printf("%s \n",command);   
            strcat(command,firstDuration);
            strcat(command,whitespace);
            printf("%s \n",command);            

            strcat(command,secondDuration);
            strcat(command,whitespace);
            printf("%s\n",secondsToStr);            
            printf("%s \n",command);
			break;
		default:
			break;
	}
    printf("TIPO\tNAME\tOPR\tInicio\tDuracao");
    printf("%d\t%s\t%s\t%d\t%d\t\n",type, fileName, opr, firstDuration, secondDuration);
    printf("? %s ?\n",command);	
    printf("-------------------------------------------\n");
        
    return command;
}

int main(void){
    int fpFIFO;
    char ch;
    FILE * arq;    
    int timer = 0;         
    char * command;
    if (access(FIFO, F_OK) == -1) {
        if (mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0) {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
            return -1; 
        } 
    } 
    puts ("Abrindo FIFO");
    if ((fpFIFO = open (FIFO, OPENMODE)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2; 
    } 
    puts ("Começando a ler...");
    arq = fopen("desc.txt","r");    
    
    
    
    while (timer <= 3){
        printf("%d \n",timer);
        command = Interpreter(arq,timer);
        printf("LEN COMMAND %d\n",strlen(command));
        write(fpFIFO,command,strlen(command));    
        printf("\n COMANDO %s",command);        
        sleep(1);
        timer += 1; 
    } 
   
    
    puts ("Fim da escrita");
    close (fpFIFO);
    return 0;
} 

    

