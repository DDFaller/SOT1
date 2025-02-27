#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Interpretador.h"
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
#define FIFO "comunicaInterToEscal"
#define FIFOINT "comunicaInter"
#define OPENMODEW O_WRONLY
#define OPENMODER O_RDONLY | O_NONBLOCK

#define COMMANDSIZE 30

static int kernelPause = 1;

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
    //printf("Palavra lida %s ultimo caracter ?%c? \n",name,c);
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
    printf("%s \n",cmp); 
    if (strcmp(cmp, "Run") == 0 || strcmp(cmp, "run")== 0) {
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
			printf("N�o foi informado a dura��o 1 para execu��o do %s \n", t_fileName);
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
			printf("N�o foi informado a dura��o 2 para execu��o do %s \n", t_fileName);
			return Unknown;
		}
        		
        *secondDuration = t_secondDuration;
        printf("Second Duration> %s\n",*secondDuration);		
        return RealTime;
	}	
    return Unknown;
}
char * Interpreter(FILE * f, int seconds, Escalonadores * caso) {
	char * palavra;
	char * fileName;
	char * opr;
    char * command;
    char secondsToStr[2];	
    char whitespace[] = {' ','\0'};
    char * firstDuration;
	char * secondDuration;
    command = (char*)malloc(sizeof(char)*COMMANDSIZE);
	Escalonadores type;
	type = ReadLine(f,&fileName,&opr,&firstDuration,&secondDuration);
    
    for (int i = 0;i < COMMANDSIZE-1;i++){
        command[i] = ' ';
    }
    command[COMMANDSIZE-1] = '\0';

    secondsToStr[0] = seconds + 48;
    secondsToStr[1] = '\0';

    strcpy(command,secondsToStr);
	switch (type)
	{
		case(prioridade):
            printf("Adicionando processo  na fila de Prioridade \n");		
            strcat(command," Priority");
            //printf("%s \n",command);            
            strcat(command,fileName);
            strcat(command,whitespace);
            //printf("%s \n",command);      
            strcat(command,firstDuration);
            strcat(command,whitespace);
            //printf("%s \n",command);            
            break;
		case(roundRobin):
            printf("Adicionando processo  na fila de Round Robin \n");
			strcat(command," RoundRobin");
            //printf("%s \n",command);           
            strcat(command,fileName);
            //printf("%s \n",command);
            break;
		case(RealTime):
            printf("Adicionando processo na fila de Real Time \n");
			strcat(command," RealTime");
            //printf("%s \n",command);            
            strcat(command,fileName);
            strcat(command,whitespace);
            //printf("%s \n",command);   
            strcat(command,firstDuration);
            strcat(command,whitespace);
            //printf("%s \n",command);            

            strcat(command,secondDuration);
            strcat(command,whitespace);
            //printf("%s\n",secondsToStr);            
            //printf("%s \n",command);
			break;
		default:
			break;
	}
    //printf("TIPO\tNAME\tOPR\tInicio\tDuracao");
    //printf("%d\t%s\t%s\t%d\t%d\t\n",type, fileName, opr, firstDuration, secondDuration);
    *caso = type;
        
    return command;
}

int main(void){
    int escFIFO;
    int kFIFO;
    char ch;
    FILE * arq;
    char kernel[10];            
    char * command;
    int timer = 0;
    Escalonadores escalonadorType;
    
    kernelPause = 1;
    
    //#define FIFO "comunicaInterToEscal"
	//#define FIFOINT "comunicaInter"

    //if (access(FIFOINT, F_OK) == -1) {
    //    if (mkfifo (FIFOINT, S_IRUSR | S_IWUSR) != 0) {
    //        fprintf (stderr, "Erro ao criar FIFO %s\n", FIFOINT);
    //        return -1; 
    //    } 
    //} 
    //puts ("Abrindo fifo Kernel - Interpretador");
    
    if (access(FIFO, F_OK) == -1) {
        if (mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0) {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
            return -1; 
        } 
    } 
    puts ("Abrindo fifo Interpretador - Escalonador");
    
    if ((escFIFO = open (FIFO, OPENMODEW)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2; 
    } 

    if ((kFIFO = open (FIFOINT, OPENMODER)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2; 
    } 
    
   	
   	
   

    puts ("Come�ando a ler...");
    arq = fopen("desc.txt","r");    
    
    while (1){
    	if(read(kFIFO,&kernel,sizeof(kernel))>0){
      		if(strcmp(kernel,"pause") == 0){
      			kernelPause = 1;
      		}
      		if(strcmp(kernel,"resume") == 0){
      			kernelPause = 0;
      		}          
      		if(strcmp(kernel,"show") == 0){
      			
      		}
     	}
        else if(kernelPause == 0){
        	printf("%d \n",timer);
        	command = Interpreter(arq,timer,&escalonadorType);
        	if(escalonadorType == Unknown){
            	break;        
        	}
        	printf("\n LEN COMMAND %d\n",strlen(command));
       		write(escFIFO,command,COMMANDSIZE);    
        	printf("\n COMANDO %s\n",command);        
        	sleep(1);
        	timer += 1; 
        	free(command);
        }
    } 
   
    
    puts ("Fim da escrita");
    close (escFIFO);
    close(kFIFO);
    return 0;
} 

    

