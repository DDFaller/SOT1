#include <stdio.h>

#include <stdlib.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <fcntl.h>

#include <unistd.h>

#define OPENMODEREAD O_RDONLY

#define OPENMODEWRITE O_WRONLY

#define LEITURA "minhaFifo3"

#define ESCRITA "minhaFifo4"

#define FIFOCreation S_IRUSR | S_IWUSR



int main(int argc, char *argv[ ] ){

	int pid,status;

	char ch;

	int fifoRead,fifoWrite;

	if( access(LEITURA,F_OK) == -1)

	{

		if(mkfifo(LEITURA,FIFOCreation) != 0){

			puts("Erro ao criar fifo");

			return -1;

		}

	}

	puts("Leitura checked");

	if( access(ESCRITA,F_OK) == -1)

	{

		if(mkfifo(ESCRITA,FIFOCreation) != 0){

			puts("Erro ao criar fifo");

			return -1;

		}

	}

	puts("Escrita checked");

	if((fifoRead = open(LEITURA,OPENMODEREAD)) < 0)

	{

		fprintf(stderr,"Erro ao abrir fifo");

		return -2;

	}

	puts("FIFO de leitura aberta...");

	if((fifoWrite = open(ESCRITA,OPENMODEWRITE)) < 0)

	{

		fprintf(stderr,"Erro ao abrir fifo");

		return -2;

	}

	puts("FIFO de escrita aberta...");

	while(1){

		if(read(fifoRead,&ch,sizeof(char)) > 0){

			printf("%d to %d \n",ch,ch -32);

			if(ch >= 97 && ch <=122){

				ch = ch - 32;

				write(fifoWrite,&ch,sizeof(char));

			}

		}

	

	}

}

