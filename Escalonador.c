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
#define FIFO "comunicaInterToEscal"
#define FIFOESC "comunicaEscal"
#define OPENMODE O_RDONLY | O_NONBLOCK
#define COMMANDSIZE 30

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
	float inicio;
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
	int tempoPorPrioridade;
	int maxPrioridades;
	int tempoParaRodar;
}Constantes;



typedef struct debugger{
	LIS_tppLista processosConcluidos;
	LIS_tppLista processosIncompletos;
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
static int kernelPause = 0;


void priority(char * fileName, int priority) {
	Processo * novoProcesso;
	novoProcesso = (Processo*)malloc(sizeof(Processo));
	
    novoProcesso->prioridade = priority;
    novoProcesso->fileName = (char*)malloc(sizeof(char) * strlen(fileName));    
    strcpy(novoProcesso->fileName,fileName);
	novoProcesso->inicio = 1000;
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
	novoProcesso->inicio = 1000;
	novoProcesso->duracao = configEscalonador->duracaoRoundRobin;

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
	novoProcesso->duracao = (float)duration;
    //printf("Real time adicionando %d\n",novoProcesso->duracao);
    novoProcesso->fileName = (char*)malloc(sizeof(char) * strlen(fileName));
	strcpy(novoProcesso->fileName,fileName);
	novoProcesso->tipo = 2;
	novoProcesso->prioridade = NULL;
    
	AdicionaProcesso(filaDeRealTime,novoProcesso);
}

void PausaProcesso(Processo * p){
    int killStatus;//0:Enviou sinal 1:Falhou
	char c;
	if( p->tipo == 0){
		if(p->prioridade < configEscalonador->maxPrioridades && kernelPause == 0){
            p->prioridade = p->prioridade +  1;
        }
	}
	if(p->tipo == 2){
		if(timeAtual < p->inicio + p->duracao && kernelPause == 0){
			printf("Erro processo %s,em Real Time foi pausado quando não deveria",p->fileName);
		}
		else{
			p->status = 2;
		}
	}
	if(p->tipo == 1){
	
	}
    
	p->status = 0;
	
    killStatus = kill(p->id,SIGSTOP);
    
    if(killStatus == 1 || p->tipo == RealTime){//Envio de sinal falhou supomos que o processo terminou
		Processo * temp;
        char ch;
        
        if(p->tipo == prioridade){
			void * output;
			
			temp = BuscaProcessoID(filaDePrioridade,p->id);
			LIS_ExcluirElementoOutput(filaDePrioridade,&output);
			LIS_InserirElementoFim(debugger->processosConcluidos,output);
		}
		
		if(p->tipo == roundRobin){
			void * output;
			
			SalvaCorrente(filaDeRoundRobin);
			
			temp = BuscaProcessoID(filaDeRoundRobin,p->id);
			LIS_ExcluirElementoOutput(filaDeRoundRobin,&output);  
			LIS_InserirElementoFim(debugger->processosConcluidos, output);
			
            ResetaCorrente(filaDeRoundRobin);
		}
		
		if(p->tipo == RealTime){
			void * output;			
            Processo * leitor;
            
            kill(p->id,SIGKILL);
                        
            temp = BuscaProcessoID(filaDeRealTime,p->id);
			LIS_ExcluirElementoOutput(filaDeRealTime,&output);
            LIS_InserirElementoFim(debugger->processosConcluidos,output);
		
		}
		
	}
}

void LiberaProcesso(Processo * p){
        
    if( p->tipo == 0){//prioridade
        p->duracao = (float)(p->prioridade * configEscalonador->tempoPorPrioridade);
    }
	if( p->tipo == 1){//roundRobin
		p->duracao = configEscalonador->duracaoRoundRobin;
	}
	if(p->tipo == 2){//RealTime
		
	}
    processoExecutando = p;
    processoExecutando->inicio = timeAtual;    
    printf("Liberando %d /%s/ %f %f",p->prioridade,p->fileName,p->inicio,p->duracao);
    
    
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
        printf("ProcessoExecutando deveria ser nulo \n");
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
    printf("Time atual %f \n",timeAtual);    
    //printf("Atualizando processo filaDeRealTime %d\n",LIS_TamanhoLista(filaDeRealTime));
    //printf("Atualizando processo filaDePrioridade %d\n",LIS_TamanhoLista(filaDePrioridade));
    //printf("Atualizando processo filaDeRoundRobin %d\n",LIS_TamanhoLista(filaDeRoundRobin));
    
    if(processoExecutando != NULL){
        if((timeAtual - processoExecutando->inicio) < processoExecutando->duracao){
            //printf("%f - %f < %f \n",timeAtual,processoExecutando-> inicio,processoExecutando-> duracao);            
            return;
        }
        else{
            PausaProcesso(processoExecutando);
            processoExecutando = NULL;
        }
    }
    
    if(processoExecutando == NULL){
        if(LIS_TamanhoLista(filaDeRealTime) && BuscaProcessoTempo(filaDeRealTime,timeAtual)){
            printf("Fila de RealTime possui elemento valido a ser executado %d\n",timeAtual);            
            EscalonaRealTime();    
        }
        else if(LIS_TamanhoLista(filaDePrioridade)){
            EscalonaPrioridade();
        
        }
        else if(LIS_TamanhoLista(filaDeRoundRobin)){
            EscalonaRoundRobin();
        }
    }
    //printf("Fila de Real Time \n");
    //ExibeProcessos(filaDeRealTime);
    //printf("Fila de Prioridade \n");
    //ExibeProcessos(filaDePrioridade);
    //printf("Fila de Round Robin \n");
    //ExibeProcessos(filaDeRoundRobin);
    if(processoExecutando != NULL){    
        //printf("%f - %f < %f \n",timeAtual,processoExecutando-> inicio,processoExecutando-> duracao);
    }    
    printf("%d \n",LIS_TamanhoLista(filaDePrioridade));
}

void ExibeProcessos(LIS_tppLista pLista){
    Processo * p;  
    for(int i =0; i < LIS_TamanhoLista(pLista);i++){
            p = (Processo*)LIS_ObterValor(pLista);
            printf("%d \t %s \t %f\t %f \n",p->id,p->fileName,p->inicio,p->duracao);
            LIS_AvancarElemento(pLista);    
    }
    printf("\n---------------FIM da Exibicao de processos------------------\n");
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
	kernelPause = 1;
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
	debugger->processosIncompletos = LIS_CriarLista(ExcluiProcesso);
	//signal(SIGSTOP,ParaProcessos)
}

Processo * BuscaProcessoID(LIS_tppLista pLista,int id){
	Processo * atual;
	
	for(int i = 0;i< LIS_TamanhoLista(pLista);i++){
		atual = (Processo*)LIS_ObterValor(pLista);
		
		if( atual->id == id){
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
	Processo * prioritario;
    for(int i =0;i < LIS_TamanhoLista(pLista);i++){
        atual = (Processo*)LIS_ObterValor(pLista);
        if(atual->prioridade < minPri){
            minPri = atual->prioridade;        
			prioritario = atual;
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

void ParaProcessos(LIS_tppLista pLista) {
	Processo * temp;
	int max;
	max = LIS_TamanhoLista(pLista);
	
	IrInicioLista(pLista);
	for(int i = 0;i< max;i++){
		void * output;
		temp = (Processo*)LIS_ObterValor(pLista);
		
		kill(temp->id,SIGKILL);
		
		LIS_ExcluirElementoOutput(pLista,&output);
		LIS_InserirElementoFim(debugger->processosIncompletos,(void*)temp);
		LIS_AvancarElemento(pLista);
	}
}

void delay(int milliseconds);

int main(void){
    int fpFIFO;
    int kFIFO;
    char ch[COMMANDSIZE];
    char kernel[10];
    int timer = 0,pos = 0,whiteSpace = 0;
    char * caso,*fileName,*inicio,*duracao;   
    init();    
    
    //#define FIFO "comunicaInterToEscal"
	//#define FIFOESC "comunicaEscal"
    //if (access(FIFO, F_OK) == -1) {
    //    if (mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0) {
    //        fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
    //        return -1;
    //    }
    //} 
    puts ("Abrindo Comunica Interpretador - Escalonador");
    
    //if (access(FIFOESC, F_OK) == -1) {
     //   if (mkfifo (FIFOESC, S_IRUSR | S_IWUSR) != 0) {
     //       fprintf (stderr, "Erro ao criar FIFO %s\n", FIFOESC);
     //       return -1;
     //   }
    //} 
    puts ("Abrindo Comunica Kernel - Escalonador");
    
    if ((kFIFO = open (FIFOESC, OPENMODE)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFOESC);
        return -2;
    }    

    if ((fpFIFO = open (FIFO, OPENMODE)) < 0) {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2;
    }
    
    
     
    puts ("Começando a ler...");
    //while  (read (fpFIFO, &ch, sizeof(ch)) > 0){
    while(timeAtual < 4){
      
      if(read(fpFIFO,&kernel,sizeof(kernel)) > 0){
      	if(strcmp(kernel,"pause") == 0){
      		kernelPause = 1;
      		PausaProcesso(processoExecutando);
      	}
      	if(strcmp(kernel,"resume") == 0){
      		kernelPause = 0;
      		LiberaProcesso(processoExecutando);
      	}
      	if(strcmp(kernel,"show") == 0){
      		printf("----------------- Fila de Prioridade ---------------");
      		ExibeProcessos(filaDePrioridade);
      		printf("----------------- Fila de Round Robin ---------------");
      		ExibeProcessos(filaDeRoundRobin);
      		printf("----------------- Fila de Real Time ---------------");
      		ExibeProcessos(filaDeRealTime);
      		ExibeProcessos(debugger->processosConcluidos);
      	}
      }
          
      if( (timeAtual -(int)timeAtual) == 0  && read(fpFIFO,&ch,sizeof(ch))> 0 && kernelPause == 0){
        pos = 0;
        printf("Comando recebido > %s \n",ch);
       
        //TIMER
        whiteSpace = FindWhiteSpace(ch,0);
        timer = atoi(GetSubstring(ch,0,whiteSpace));
        pos = whiteSpace;
        
        if(timer == -1){
            whiteSpace = FindWhiteSpace(ch,pos + 1);
            kernel = GetSubstring(ch,pos,whiteSpace);
            pos = whiteSpace;          
            printf("Comando %s \n",kernel);
            
            if(strcmp(kernel,"pause") == 0){
      		    kernelPause = 1;
      		    PausaProcesso(processoExecutando);
      	    }
      	    if(strcmp(kernel,"resume") == 0){
      		    kernelPause = 0;
      		    LiberaProcesso(processoExecutando);
      	    }
      	    if(strcmp(kernel,"show") == 0){
      		    printf("----------------- Fila de Prioridade ---------------");
      		    ExibeProcessos(filaDePrioridade);
      		    printf("----------------- Fila de Round Robin ---------------");
      		    ExibeProcessos(filaDeRoundRobin);
      		    printf("----------------- Fila de Real Time ---------------");
      		    ExibeProcessos(filaDeRealTime);
      		    ExibeProcessos(debugger->processosConcluidos);
      	    }         
                                                                        
        }
        else{
        
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
        printf("Comando limpo > %s \n",ch);
      }
      strcpy(ch," \0");

      if(kernelPause == 0){
        AtualizaProcesso();
      }
      
      delay(500);
    }
    IrInicioLista(filaDeRoundRobin);
    LIS_AvancarElemento(filaDeRoundRobin);
    LIS_AvancarElemento(filaDeRoundRobin);
    LIS_AvancarElemento(filaDeRoundRobin);
    
    
    printf("-----------------Processos Concluidos--------------------------\n");
    ExibeProcessos(debugger->processosConcluidos);
    ParaProcessos(filaDePrioridade);
    ParaProcessos(filaDeRoundRobin);
    ParaProcessos(filaDeRealTime);
    
    printf("----------------------Processos Incompletos-------------------------\n");
    ExibeProcessos(debugger->processosIncompletos);
    //ExibeProcessos(filaDeRealTime);
    //ExibeProcessos(filaDePrioridade);
    //ExibeProcessos(filaDeRoundRobin);
    puts ("Fim da leitura");
    close (fpFIFO);
    close (kFIFO);
    return 0;
}

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause ){
        now = clock();
    }
    if(kernelPause == 0){
    	timeAtual += 0.5;
    }
}
