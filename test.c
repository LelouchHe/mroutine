#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

int main()
{
    ucontext_t context;
    int i;

    getcontext(&context);
    for (i = 0; i < 1000000; i++)
        printf("Hello world");
    sleep(1);
    setcontext(&context);
    return 0;
}
