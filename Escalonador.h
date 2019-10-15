
typedef enum
{
	prioridade,
	roundRobin,
	RealTime,
	Unknown
}Escalonadores;

void priority(char * fileName,int priority);
void roundrobin(char * fileName );
void realTime(char * fileName, int init,int duration);