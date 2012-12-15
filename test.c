#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "mroutine.h"

void test_fun(struct mroutine_t *mr, int a)
{
    a = mr_yield(mr, a + 1);
}

void test(struct mroutine_t *mr, int num)
{
    int a = 1;
    int b = 100;
    mid_t a_mid = mr_create(mr, test_fun);
    mid_t b_mid = mr_create(mr, test_fun);
    
    int i = 0;
    void *ret = NULL;
    while (i++ < num)
    {
        ret = mr_resume(mr, a_mid, a++);
        ret = mr_resume(mr, b_mid, b++);
    }
}

int main(int argc, char *argv[])
{
    int num = atoi(argv[1]);
    struct mroutine_t *mr = mr_ini(0, 0);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    test(mr, num);
    gettimeofday(&end, NULL);

    printf("num: %d\ncost: %d(us)\n", num,
            (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
    
    mr_fini(mr);

    return 0;
}
