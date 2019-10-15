#include "Escalonador.h"
#include "Lista.h"

#define NULL 0

static LIS_tppLista processos;

typedef struct processo {
	Escalonadores tipo;
	int inicio;
	int duracao;
	int prioridade;
	//Comum para todos
	int id;
	char * fileName;
} Processo;

void priority(char * fileName, int priority) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));
	
	novoProcesso->prioridade = priority;
	novoProcesso->fileName = fileName;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = NULL;
	novoProcesso->tipo = prioridade;

	AdicionaProcesso(novoProcesso);
}
void roundrobin(char * fileName) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));

	novoProcesso->tipo = roundRobin;
	novoProcesso->fileName = fileName;
	novoProcesso->prioridade = NULL;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = NULL;

	AdicionaProcesso(novoProcesso);
}
void realTime(char * fileName, int init, int duration) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));

	novoProcesso->inicio = init;
	novoProcesso->duracao = duration;
	novoProcesso->fileName = fileName;
	novoProcesso->tipo = RealTime;
	novoProcesso->prioridade = NULL;

	AdicionaProcesso(novoProcesso);
}

void AdicionaProcesso(Processo * p) {
	LIS_InserirElementoFim(processos, (void*)p);
}

void init() {
	//segmento1 = shmget(8752, sizeof(int) * 4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	processos = LIS_CriarLista(ExcluiProcesso);
	//signal(SIGSTOP,ParaProcessos)

}

void ExcluiProcesso(void * pDado) {
	Processo * atual;
	atual = (Processo*)pDado;
	free(atual->fileName);
	free(pDado);
}

void ParaProcessos(int signo) {
	Processo * p = (Processo*)LIS_ObterValor(processos);
	//kill(p->id, SIGSTOP);
}