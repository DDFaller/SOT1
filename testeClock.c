#include <stdio.h>
#include <time.h>

void delay(int milliseconds);

int main()
{
    int x;

    puts("Loop with 250 millisecond delay:");
    for(x=0;x<10;x++)
    {
        printf("%d\n",x+1);
        delay(1000);
    }
    putchar('\n');

    return(0);
}

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}
