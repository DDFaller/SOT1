#include <stdio.h>

#include <stdlib.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <fcntl.h>

#include <unistd.h>

#define OPENMODEREAD O_RDONLY

#define OPENMODEWRITE O_WRONLY

#define ESCRITA "minhaFifo3"

#define LEITURA "minhaFifo4"

#define FIFOCreation S_IRUSR | S_IWUSR



int main(int argc, char *argv[ ] ){

	int pid,status;

	char ch,c;

	int fifoRead,fifoWrite;

	if( access(LEITURA,F_OK) == -1)

	{

		if(mkfifo(LEITURA,FIFOCreation) != 0){

			puts("Erro ao criar fifo");

			return -1;

		}

	}

	if( access(ESCRITA,F_OK) == -1)

	{

		if(mkfifo(ESCRITA,FIFOCreation) != 0){

			puts("Erro ao criar fifo");

			return -1;

		}

	}

	if((fifoWrite = open(ESCRITA,OPENMODEWRITE)) < 0)

	{

		fprintf(stderr,"Erro ao abrir fifo");

		return -2;

	}

	if((fifoRead = open(LEITURA,OPENMODEREAD)) < 0)

	{

		fprintf(stderr,"Erro ao abrir fifo");

		return -2;

	}

	

	while(1){

		ch = argv[1];

		write(fifoWrite,&ch,sizeof(char));

		while (read (fifoRead, &c, sizeof(char)) > 0) printf("%c\n",c); 

	

	}

}

