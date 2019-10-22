#include <fcntl.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#define FIFOINT "comunicaInter"
#define FIFOESC	"comunicaEscal"
#define OPENMODEN O_WRONLY
#define OPENMODE O_WRONLY
#define TRUE 1
#define COMMANDSIZE 10

int main(int argc,char * argv[]){
    int escFIFO;  
    int intFIFO;
    char command[COMMANDSIZE];
    //#define FIFOINT "comunicaInter"
	//#define FIFOESC	"comunicaEscal"
    if (access(FIFOINT, F_OK) == -1) {
       if (mkfifo (FIFOINT, S_IRUSR | S_IWUSR) != 0) {
           fprintf (stderr, "Erro ao criar FIFO %s\n", FIFOINT);
           return -1;
       }
    } 
    puts ("Abrindo FIFO Interpretador");
    
    if (access(FIFOESC, F_OK) == -1) {
       if (mkfifo (FIFOESC, S_IRUSR | S_IWUSR) != 0) {
           fprintf (stderr, "Erro ao criar FIFO %s\n", FIFOESC);
           return -1;
       }
    } 
    puts ("Abrindo FIFO Escalonador");
    
    if ((escFIFO = open (FIFOINT, OPENMODE)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFOINT);
        return -2;
    }
    
    if ((intFIFO = open (FIFOESC, OPENMODEN)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFOESC);
        return -2;
    }
     
    while(TRUE){
        printf("Digite seu comando\n");
        scanf("%s",command);
        printf("%s\n",command);
        
        write(intFIFO,command,COMMANDSIZE);		
        write(escFIFO,command,COMMANDSIZE);	
	
		      
    }



    return 0;
}
