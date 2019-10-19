#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO "minhaFifo"
#define OPENMODE O_RDONLY

int main (void) {
    int fpFIFO;
    char ch[80];
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
    while (read (fpFIFO, &ch, sizeof(ch)) > 0) printf("%s \n",ch);
    puts ("Fim da leitura");
    close (fpFIFO);
    return 0;
}
