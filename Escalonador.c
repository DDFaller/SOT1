#include "Escalonador.h"
#include "Lista.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>

#define NULL 0

static LIS_tppLista filaDeRoundRobin;
static LIS_tppLista filaDePrioridade;
static LIS_tppLista filaDeRealTime;

static int timeAtual;

const char * vectorNull = {NULL};

typedef struct processo {
	int inicio;
	float duracao;
	int prioridade;
	//Comum para todos
    Escalonadores tipo;
	int id;
	char * fileName;
    int status;
} Processo;

typedef struct debugger{
    int qtdConcluidos;
}Debugger;


static Processo * processoExecutando;

void priority(char * fileName, int priority) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));
	
	novoProcesso->prioridade = priority;
	novoProcesso->fileName = fileName;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = prioridade * 2;
	novoProcesso->tipo = prioridade;

	AdicionaProcesso(filaDePrioridade,novoProcesso);
}
void roundrobin(char * fileName) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));

	novoProcesso->tipo = roundRobin;
	novoProcesso->fileName = fileName;
	novoProcesso->prioridade = NULL;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = 0.5;

	AdicionaProcesso(filaDeRoundRobin,novoProcesso);
}
void realTime(char * fileName, int init, int duration) {
    if( init + duration > 60){
        printf("Nao e possivel adicionar esse processo, excede o tempo permitido \n");
        return;
    }	
    Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));

	novoProcesso->inicio = init;
	novoProcesso->duracao = duration;
	novoProcesso->fileName = fileName;
	novoProcesso->tipo = RealTime;
	novoProcesso->prioridade = NULL;
    
	AdicionaProcesso(filaDeRealTime,novoProcesso);
}

void PausaProcesso(Processo * p){
    kill(SIGSTOP,p->id);
}

void LiberaProcesso(Processo * p){
    processoExecutando->inicio = tempoAtual;    
    processoExecutando = p;
    kill(SIGCONT,p->id);    
    
}

void EscalonaRealTime(){
    Processo * pendente;
    pendente = BuscaProcessoTempo(filaDeRealTime,tempoAtual);    
    if(processoExecutando == NULL){
        LiberaProcesso(pendente); 
    }
    else{
        if(tempoAtual - processoExecutando->inicio > processoExecutando->duracao){
            PausaProcesso(processoExecutando);
            LiberaProcesso(pendente);
        }
        else{
            printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecuntando->fileName,pendente->fileName,timeAtual);
        }
    }
    
}

void EscalonaPrioridade(){
    Processo * pendente;
    pendente = BuscaProcessoTempo(filaDeRealTime,tempoAtual);    
    if(processoExecutando == NULL){
        LiberaProcesso(pendente); 
    }
    else{
        if(tempoAtual - processoExecutando->inicio > processoExecutando->duracao){
            PausaProcesso(processoExecutando);
            LiberaProcesso(pendente);
        }
        else{
            printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecuntando->fileName,pendente->fileName,timeAtual);
        }
    }
}

void EscalonaRoundRobin(){
}

void ExibeProcessos(LIS_tppLista pLista){
    Processo * p;  
    printf("---------------Exibicao de processos------------------");  
    for(int i =0; i < LIS_TamanhoLista(pLista);i++){
            p = (Processo*)LIS_ObterValor(pLista);
            printf("%d \t %s \t %d \n",p->id,p->fileName,p->inicio);
            LIS_AvancarElemento(pLista);    
    }
    printf("---------------FIM da Exibicao de processos------------------");
}


void AtualizaProcesso(){
    Processo * temp;
    timeAtual += 1;
    if(LIS_TamanhoLista(filaDeRealTime)){
        EscalonaRealTime();    
    }
    else if(LIS_TamanhoLista(filaDePrioridade)){
        EscalonaPrioridade();    
    }
    else{
        EscalonaRoundRobin();
    }
    printf("Fila de Real Time \n");
    ExibeProcessos(filaDeRealTime);
    printf("Fila de Prioridade \n");
    ExibeProcessos(filaDePrioridade);
    printf("Fila de Round Robin \n");
    ExibeProcessos(filaDeRoundRobin);
}

void AdicionaProcesso(LIS_tppLista pLista,Processo * p) {
    int segmento;
    int * pid;	
    segmento = shmget(8752, sizeof(int) * 4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    pid = (int*)shmat(segmento,0,0);
    *pid = 0;    
    if(fork() < 0){
        *pid = getpid();
        kill(SIGSTOP,getpid());
2000
        execve(p->fileName,vectorNull,NULL);
    }
    else{
        while(pid == 0){
        }
        p->id = *pid;
        LIS_InserirElementoFim(pLista, (void*)p);       
        AtualizaProcesso();    
    }
}

void ExcluiProcesso(void * pDado) {
	Processo * atual;
	atual = (Processo*)pDado;
	free(atual->fileName);
	free(pDado);
}

void init() {
    timeAtual = 0;
	//segmento1 = shmget(8752, sizeof(int) * 4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    
    filaDePrioridade = LIS_CriarLista(ExcluiProcesso); 
    filaDeRoundRobin = LIS_CriarLista(ExcluiProcesso);
    filaDeRealTime   = LIS_CriarLista(ExcluiProcesso);
	//signal(SIGSTOP,ParaProcessos)
}

Processo * BuscaProcessoTempo(LIS_tppLista pLista, int time){
    Processo * atual;    
    for(int i =0;i < LIS_TamanhoLista(pLista);i++){
        atual = (Processo*)LIS_ObterValor(pLista);            
        if(atual->inicio == time){
            return atual;        
        }    
        LIS_AvancarElemento(pLista);
    }
    return NULL;    
}

void ParaProcessos(int signo) {
	
	//kill(p->id, SIGSTOP);
}
