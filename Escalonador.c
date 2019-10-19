#include "Escalonador.h"
#include "Lista.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#if defined TIMEFUNCTION
#include <time.h>
#else
#endif


#define NULL 0

static LIS_tppLista filaDeRoundRobin;
static LIS_tppLista filaDePrioridade;
static LIS_tppLista filaDeRealTime;

#if defined TIMEFUNCTION
static time_T timeAtual;	
#else
static int timeAtual;
#endif

const char * vectorNull = {NULL};

typedef struct processo {
	int inicio;
	float duracao;
	int prioridade;
	//Comum para todos
    Escalonadores tipo;
	int id;
	char * fileName;
    int status;//0: Bloqueado 1: Executando 2:Finalizado
} Processo;

typedef struct constantes{
	float duracaoRoundRobin;
	float tempoPorPrioridade;
	int maxPrioridades;
	int tempoParaRodar;
}Constantes;



typedef struct debugger{
	LIS_tppLista processosConcluidos;
}Debugger;


static Constantes * configEscalonador;
static Debugger * debugger;
static Processo * processoExecutando;



void priority(char * fileName, int priority) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo*));
	
	novoProcesso->prioridade = priority;
	novoProcesso->fileName = fileName;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = prioridade * configEscalonador->tempoPorPrioridade;
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
	novoProcesso->duracao = configEscalonador->tempoParaRodar;

	AdicionaProcesso(filaDeRoundRobin,novoProcesso);
}
void realTime(char * fileName, int init, int duration) {
    if( init + duration > configEscalonador->tempoParaRodar){
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
    int killStatus;//0:Enviou sinal 1:Falhou
	
	if( p->tipo == Prioridade){
		p->prioridade += 1;
	}
	if(p->tipo == RealTime){
		if(timeAtual != p->inicio + p->duracao){
			printf("Erro processo %s,em Real Time foi pausado quando não deveria",p->fileName);
		}
		else{
			p->status = 2;
		}
	}
	if(p->tipo == roundRobin){
	
	}
	p->status = 0;
	killStatus = kill(p->id,SIGSTOP);
	
	if(killStatus == 1){//Envio de sinal falhou supomos que o processo terminou
		processo * p;
		if(processoExecutando->tipo == prioridade){
			void * output;
			p = BuscaProcessoID(filaDePrioridade,processoExecutando->id);
			LIS_ExcluirElemento(filaDeRoundRobin,&output);
			LIS_InserirElementoFim(debugger->processosConcluidos,output);
		}
		if(processoExecutando->tipo == RealTime){
			void * output;
			p = BuscaProcessoID(filaDePrioridade,processoExecutando->id);
			LIS_ExcluirElemento(filaDeRoundRobin,&output);
			LIS_InserirElementoFim(debugger->processosConcluidos,output);
		
		}
		if(processoExecutando->tipo == roundRobin){
			void * output;
			SalvaCorrente(filaDeRoundRobin);
			
			p = BuscaProcessoID(filaDeRoundRobin,processoExecutando->id);
			LIS_ExcluirElemento(filaDeRoundRobin,&output);  
			LIS_InserirElementoFim(debugger->processosConcluidos, output);
			ResetaCorrente(filaDeRoundRobin);
		}
	}
	
}

void LiberaProcesso(Processo * p){
	if( p->tipo == prioridade){
		p->duracao = p->prioridade * configEscalonador->tempoPorPrioridade;
	}
	if( p->tipo == roundrobin){
		p->duracao = configEscalonador->duracaoRoundRobin;
	}
	if(p->tipo == RealTime){
		
	}

    processoExecutando->inicio = tempoAtual;    
    processoExecutando = p;
	p->status = 1;
	kill(p->id,SIGCONT);       
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
            printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecutando->fileName,pendente->fileName,timeAtual);
        }
    }
}

void EscalonaPrioridade(){
    Processo * pendente;
    pendente = BuscaProcessoPrioritario(filaDeRealTime);    
   
    if(processoExecutando == NULL){
        LiberaProcesso(pendente); 
    }
    else{
        if(tempoAtual - processoExecutando->inicio > processoExecutando->duracao){
            PausaProcesso(processoExecutando);
            LiberaProcesso(pendente);
        }
        else{
            printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecutando->fileName,pendente->fileName,timeAtual);
        }
    }
}

void EscalonaRoundRobin(){
	Processo * pendente;
	pendente = (Processo*)LIS_ObterValor(filaDeRoundRobin);
	
	if(pendente == NULL){
		return;
	}

	if(processoExecutando == NULL){
		LiberaProcesso(pendente);
	}
	else{
		if(tempoAtual - processoExecutando->inicio > processoExecutando->duracao){
			PausaProcesso(processoExecutando);
			LiberaProcesso(pendente);
		}
		else{
			printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecutando->fileName,pendente->fileName,timeAtual);
		}
	}
	LIS_AvancarElemento(filaDeRoundRobin);
}

void AtualizaProcesso(){
    Processo * temp;
    timeAtual += 1;
    if(LIS_TamanhoLista(filaDeRealTime) && BuscaProcessoTempo(filaDeRealTime,timeAtual)){
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

void AdicionaProcesso(LIS_tppLista pLista,Processo * p) {
    int segmento;
    int * pid;	
    
    segmento = shmget(8752, sizeof(int) * 4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    pid = (int*)shmat(segmento,0,0);
    *pid = 0;    
    if(fork() < 0){
        *pid = getpid();
        kill(getpid(),SIGSTOP);
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
	#if defined TIMEFUNCTION
		timeAtual = time(NULL);
	#else
		timeAtual = 0;
    #endif
    processoExecutando = NULL;

    filaDePrioridade = LIS_CriarLista(ExcluiProcesso); 
    filaDeRoundRobin = LIS_CriarLista(ExcluiProcesso);
    filaDeRealTime   = LIS_CriarLista(ExcluiProcesso);
	
	configEscalonador = (Constantes*)malloc(sizeof(Constantes));
	configEscalonador->duracaoRoundRobin = 0.5;//float
	configEscalonador->maxPrioridades = 7;//int
	configEscalonador->tempoPorPrioridade = 2;//float
	configEscalonador->tempoParaRodar = 60;//int

	debugger = (Debugger*)malloc(sizeof(Debugger));
	debugger->processosConcluidos = LIS_CriarLista(ExcluiProcesso);
	//signal(SIGSTOP,ParaProcessos)
}

Processo * BuscaProcessoID(LIS_tppLista pLista,int id){
	Processo * atual;
	
	for(int i = 0;i< LIS_TamanhoLista(pLista);i++){
		atual = (Processo*)LIS_ObterValor(pLista);
		
		if( atual->id == id){
			ResetaCorrente(pLista);
			return atual;
		}
		LIS_AvancarElemento(pLista);
	}
	return NULL;
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

Processo * BuscaProcessoPrioritario(LIS_tppLista pLista){
    Processo * atual;
	int minPri = configEscalonador->maxPrioridades + 1;
	processo * prioritario = NULL;
    for(int i =0;i < LIS_TamanhoLista(pLista);i++){
        atual = (Processo*)LIS_ObterValor(pLista);            
        if(atual->Prioridade < minPri){
            minPri = atual->Prioridade;        
			prioritario = atual;
        }    
        LIS_AvancarElemento(pLista);
    }
    return prioritario;    
}

void ParaProcessos(int signo) {
	
	//kill(p->id, SIGSTOP);
}

char ReadWord(char ** msg, char ** word) {
	char c;
	int idx = 0;
	char * nameHolder = (char*)malloc(sizeof(char) * 20);
	char * name = (char*)malloc(sizeof(char) * 20);

	nameHolder[idx++] = c;
	while (TRUE) {
		c = fgetc(f);
	    if(c == ' ') {
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



int main(void){
    int fpFIFO;
    char ch[80];
    int timer = 0;
    char * caso;
    
    init();    
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
    while (read (fpFIFO, &ch, sizeof(ch)) > 0){
      time = atoi(ch[0]);
         
    }
    puts ("Fim da leitura");
    close (fpFIFO);
    return 0;

    return 0;
}
