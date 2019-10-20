#include "Lista.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include<string.h>
#include <time.h>

#define TRUE 1
#define FIFO "minhaFifo"
#define OPENMODE O_RDONLY

static LIS_tppLista filaDeRoundRobin;
static LIS_tppLista filaDePrioridade;
static LIS_tppLista filaDeRealTime;

#if defined TIMEFUNCTION
static time_T timeAtual;	
#else
static float timeAtual;
#endif

const char * vectorNull = {NULL};

typedef enum
{
	prioridade,
	roundRobin,
	RealTime,
	Unknown
}Escalonadores;

typedef struct processo {
	int inicio;
	int duracao;
	int prioridade;
	//Comum para todos
    Escalonadores tipo;
	int id;
	char * fileName;
    int status;//0: Bloqueado 1: Executando 2:Finalizado
} Processo;

typedef struct constantes{
	int duracaoRoundRobin;
	int tempoPorPrioridade;
	int maxPrioridades;
	int tempoParaRodar;
}Constantes;



typedef struct debugger{
	LIS_tppLista processosConcluidos;
}Debugger;

void priority(char * fileName,int priority);
void roundrobin(char * fileName);
void realTime(char * fileName,int inicio,int fim);
void AdicionaProcesso(LIS_tppLista pLista, Processo * p);
Processo * BuscaProcessoID(LIS_tppLista pLista,int id);
Processo * BuscaProcessoTempo(LIS_tppLista pLista,int id);
Processo * BuscaProcessoPrioritario(LIS_tppLista pLista);


static Constantes * configEscalonador;
static Debugger * debugger;
static Processo * processoExecutando;



void priority(char * fileName, int priority) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo));
	novoProcesso->prioridade = priority ;
    novoProcesso->fileName = (char*)malloc(sizeof(char) * strlen(fileName));    
    strcpy(novoProcesso->fileName,fileName);
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = priority * configEscalonador->tempoPorPrioridade;
	novoProcesso->tipo = 0;

	AdicionaProcesso(filaDePrioridade,novoProcesso);
}
void roundrobin(char * fileName) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo));

	novoProcesso->tipo = 1;
    novoProcesso->fileName = (char*)malloc(sizeof(char) * strlen(fileName));   	
    strcpy(novoProcesso->fileName,fileName);
	novoProcesso->prioridade = NULL;
	novoProcesso->inicio = NULL;
	novoProcesso->duracao = configEscalonador->tempoParaRodar;

	AdicionaProcesso(filaDeRoundRobin,novoProcesso);
}
void realTime(char * fileName, int init, int duration) {
    char ch;    
    if( init + duration > configEscalonador->tempoParaRodar){
        printf("Nao e possivel adicionar esse processo, excede o tempo permitido \n");
        return;
    }	
    Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo));

	novoProcesso->inicio = init;
	novoProcesso->duracao = duration;
    //printf("Real time adicionando %d\n",novoProcesso->duracao);
    novoProcesso->fileName = (char*)malloc(sizeof(char) * strlen(fileName));
	strcpy(novoProcesso->fileName,fileName);
	novoProcesso->tipo = 2;
	novoProcesso->prioridade = NULL;
    
	AdicionaProcesso(filaDeRealTime,novoProcesso);
}

void PausaProcesso(Processo * p){
    int killStatus;//0:Enviou sinal 1:Falhou
	
	if( p->tipo == prioridade){
		p->prioridade += 1;
	}
	if(p->tipo == RealTime){
		if(timeAtual < p->inicio + p->duracao){
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
		Processo * p;
		if(processoExecutando->tipo == prioridade){
			void * output;
			p = BuscaProcessoID(filaDePrioridade,processoExecutando->id);
			LIS_ExcluirElementoOutput(filaDePrioridade,&output);
			LIS_InserirElementoFim(debugger->processosConcluidos,output);
		}
		if(processoExecutando->tipo == RealTime){
			void * output;
			p = BuscaProcessoID(filaDeRealTime,processoExecutando->id);
			LIS_ExcluirElementoOutput(filaDeRoundRobin,&output);
			LIS_InserirElementoFim(debugger->processosConcluidos,output);
		
		}
		if(processoExecutando->tipo == roundRobin){
			void * output;
			SalvaCorrente(filaDeRoundRobin);
			
			p = BuscaProcessoID(filaDeRoundRobin,processoExecutando->id);
			LIS_ExcluirElementoOutput(filaDeRoundRobin,&output);  
			LIS_InserirElementoFim(debugger->processosConcluidos, output);
			ResetaCorrente(filaDeRoundRobin);
		}
	}
	
}

void LiberaProcesso(Processo * p){
        
    if( p->tipo == 0){//prioridade
        p->duracao = p->prioridade * configEscalonador->tempoPorPrioridade;
    }
	if( p->tipo == 1){//roundRobin
		p->duracao = configEscalonador->duracaoRoundRobin;
	}
	if(p->tipo == 2){//RealTime
		
	}
    processoExecutando = p;
    processoExecutando->inicio = timeAtual;    
    printf("/%s/ %d %d %d",p->fileName,p->inicio,p->duracao,p->prioridade);
    
    
    p->status = 1;
	kill(p->id,SIGCONT);       
}

void EscalonaRealTime(){
    Processo * pendente;
    pendente = BuscaProcessoTempo(filaDeRealTime,timeAtual);    
    
	if(processoExecutando == NULL){
        LiberaProcesso(pendente); 
    }
    else{
        if(timeAtual - processoExecutando->inicio >= processoExecutando->duracao){
            //PausaProcesso(processoExecutando);
            LiberaProcesso(pendente);
        }
        else{
            printf("Houve um conflito entre processo %s e processo %s no instante \n",processoExecutando->fileName,pendente->fileName,timeAtual);
        }
    }
}

void EscalonaPrioridade(){
    Processo * pendente;
    char ch;
    pendente = BuscaProcessoPrioritario(filaDePrioridade);    
    if(pendente == NULL){
        printf("Erro na funcao BuscaProcessoPrioritario \n");   
    }
    if(processoExecutando == NULL){        
        LiberaProcesso(pendente); 
    }
    else{
        if(timeAtual - processoExecutando->inicio >= processoExecutando->duracao){
            //PausaProcesso(processoExecutando);
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
		if(timeAtual - processoExecutando->inicio >= processoExecutando->duracao){
			//PausaProcesso(processoExecutando);
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
    char ch;    
    //printf("Time atual %d \n",timeAtual);    
    //printf("Atualizando processo filaDeRealTime %d\n",LIS_TamanhoLista(filaDeRealTime));
    //printf("Atualizando processo filaDePrioridade %d\n",LIS_TamanhoLista(filaDePrioridade));
    //printf("Atualizando processo filaDeRoundRobin %d\n",LIS_TamanhoLista(filaDeRoundRobin));
    if(processoExecutando != NULL){
        if(timeAtual - processoExecutando-> inicio < processoExecutando->duracao){
            printf("%f - %d < %d \n",timeAtual,processoExecutando-> inicio,processoExecutando-> duracao);
            printf("Processo Rodando %s %d %d\n",processoExecutando->fileName,processoExecutando->id,processoExecutando->status);
            return;
        }
        else{
            PausaProcesso(processoExecutando);
        }
    }
    
    if(LIS_TamanhoLista(filaDeRealTime) && BuscaProcessoTempo(filaDeRealTime,timeAtual)){
        EscalonaRealTime();    
    }
    else if(LIS_TamanhoLista(filaDePrioridade)){
        EscalonaPrioridade();
        
    }
    else{
        EscalonaRoundRobin();
    }

    //printf("Fila de Real Time \n");
    //ExibeProcessos(filaDeRealTime);
    //printf("Fila de Prioridade \n");
    //ExibeProcessos(filaDePrioridade);
    //printf("Fila de Round Robin \n");
    //ExibeProcessos(filaDeRoundRobin);
    printf("%f - %d < %d \n",timeAtual,processoExecutando-> inicio,processoExecutando-> duracao);
}

void ExibeProcessos(LIS_tppLista pLista){
    Processo * p;  
    printf("---------------Exibicao de processos------------------\n");  
    for(int i =0; i < LIS_TamanhoLista(pLista);i++){
            p = (Processo*)LIS_ObterValor(pLista);
            printf("%d \t %s \t %d\t %d prio> %d\n",p->id,p->fileName,p->inicio,p->duracao,p->prioridade);
            LIS_AvancarElemento(pLista);    
    }
    printf("---------------FIM da Exibicao de processos------------------\n");
}

void AdicionaProcesso(LIS_tppLista pLista,Processo * p) {
    int segmento;
    int * pid;	
    char ch;
    
    segmento = shmget(8752, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
    pid = (int*)shmat(segmento,0,0);
    *pid = 0;      
    if(fork() == 0){
        *pid = getpid();
        kill(getpid(),SIGSTOP);
        execve(p->fileName,vectorNull,NULL);
    }
    else{
        while(*pid == 0){
        }
        p->id = *pid;
        LIS_InserirElementoFim(pLista, (void*)p);       
        //AtualizaProcesso();    
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
	configEscalonador->duracaoRoundRobin = 1;//float
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
    return 0;    
}

Processo * BuscaProcessoPrioritario(LIS_tppLista pLista){
    Processo * atual;
	int minPri = configEscalonador->maxPrioridades + 1;
	Processo * prioritario = (Processo*)malloc(sizeof(Processo));
    for(int i =0;i < LIS_TamanhoLista(pLista);i++){
        atual = (Processo*)LIS_ObterValor(pLista);
        if(atual->prioridade < minPri){
            minPri = atual->prioridade;        
			*prioritario = *atual;
        }       
        if(LIS_TamanhoLista(pLista) != 1){
            LIS_AvancarElemento(pLista);
        }
    }
    return prioritario;    
}

char * GetSubstring(char * ch, int start, int end){
    char *inicio = &ch[start];
    char *fim = &ch[end];
    // Note the + 1 here, to have a null terminated substring
    char *substr = (char *)calloc(1, fim - inicio + 1);
    memcpy(substr, inicio, fim - inicio);
    return substr;
}

int FindWhiteSpace(char * palavra, int idx){
    int max;
    char ch;
    max = strlen(palavra);
    while(idx != max){
        ch = palavra[idx];
        if(ch == ' ' || ch == '\n'){
            return idx;        
        }        
        idx += 1;        
    }
    return max;
}

void ParaProcessos(int signo) {
	
	//kill(p->id, SIGSTOP);
}

void delay(int milliseconds);

int main(void){
    int fpFIFO;
    char ch[30];
    int timer = 0,pos = 0,whiteSpace = 0;
    char * caso,*fileName,*inicio,*duracao;   
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
    ExibeProcessos(filaDePrioridade);    
    puts ("Começando a ler...");
    //while  (read (fpFIFO, &ch, sizeof(ch)) > 0){
    while(1){    
      if( (timeAtual -(int)timeAtual) == 0  && read(fpFIFO,&ch,sizeof(ch))> 0){
        pos = 0;
        printf("Comando recebido > %s \n",ch);
        //TIMER
        whiteSpace = FindWhiteSpace(ch,0);
        timer = atoi(GetSubstring(ch,0,whiteSpace));
        pos = whiteSpace;
        //CASO
        whiteSpace = FindWhiteSpace(ch,pos + 1);
        caso = GetSubstring(ch,pos,whiteSpace);
        pos = whiteSpace;
      
        //FILENAME
        whiteSpace = FindWhiteSpace(ch,pos + 1);
        fileName = GetSubstring(ch,pos + 1,whiteSpace);
        pos = whiteSpace;
      
        //INICIO
        if(strcmp(caso," Priority") == 0 || strcmp(caso," RealTime") == 0){
            whiteSpace = FindWhiteSpace(ch,pos + 1);
            inicio = GetSubstring(ch,pos,whiteSpace);
            pos = whiteSpace;
            printf("Inicio  %s \n",inicio);
        }
        //DURACAO
        if(strcmp(caso," RealTime") == 0){
            whiteSpace = FindWhiteSpace(ch,pos + 1);
            duracao = GetSubstring(ch,pos,whiteSpace);
            pos = whiteSpace;
            printf("Duracao  %s \n",duracao);      
        }
 
        if(strcmp(caso," Priority") == 0){
            //printf("Priority a adicionar %s %d\n",fileName,atoi(inicio)); 
            priority(fileName,atoi(inicio));
        }
        if(strcmp(caso," RealTime") == 0){
            //printf("Realtime a adicionar %s %d %d\n",fileName,atoi(inicio),atoi(duracao));        
            realTime(fileName,atoi(inicio),atoi(duracao));
        }
        if(strcmp(caso," RoundRobin") == 0){
            //printf("RoundRobin a adicionar %s \n",fileName);         
            roundrobin(fileName);
        }
      }
      strcpy(ch," \0");
      AtualizaProcesso();
      
      delay(500);
    }
    
    //ExibeProcessos(filaDeRealTime);
    //ExibeProcessos(filaDePrioridade);
    //ExibeProcessos(filaDeRoundRobin);
    puts ("Fim da leitura");
    close (fpFIFO);
    return 0;
}

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
    timeAtual += 0.5;
}
