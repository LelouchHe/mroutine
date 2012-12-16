#include <stdio.h>

#include "mroutine.h"

int test_fun(struct mroutine_t *mr, int a)
{
    int i = 0;
    while (i++ < 5)
        printf("yield in test_fun: %d\n", mr_yield(mr, a++ + 100));
}

void test(struct mroutine_t *mr)
{
    mid_t mid = mr_create(mr, test_fun);

    int i = 0;
    while (i++ < 5)
        printf("resume in test: %d\n", mr_resume(mr, mid, i));
}

int main()
{
    struct mroutine_t *mr = mr_ini(0, 0);

    test(mr);

    mr_fini(mr);

    return 0;
}
