#include <fcntl.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#define FIFO meuKernel
#define OPENMODE O_WRONLY
#define TRUE 1
int main(int argc,char * argv[]){
    int fpFIFO;    
    
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
     
    while(TRUE){
        printf("Digite seu comando");
        
    }



    return 0;
}
