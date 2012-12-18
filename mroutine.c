#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <ucontext.h>

#include "mroutine.h"

#define DEF_STACK_SIZE (1024 * 1024)
#define MIN_STACK_SIZE (4 * 1024)
#define DEF_COROUTINE_NUM (32)

// 兼容性
#ifndef MAP_STACK
#define MAP_STACK 0x20000
#endif

struct coroutine_t
{
    mr_fun_t fun;
    void *args;
    void *ret;

    int status;

    ucontext_t prev;
    mid_t prev_mid;
    ucontext_t cur;

    char *stack;
    int stack_size;
};

struct mroutine_t
{
    int stack_size;

    struct coroutine_t **crs;
    int cr_num;
    int cr_max;
    mid_t cr_mid;
};


static struct coroutine_t *cr_ini(mr_fun_t fun, int stack_size)
{
    struct coroutine_t *cr = (struct coroutine_t *)malloc(sizeof (struct coroutine_t));
    if (cr == NULL)
        return NULL;

    cr->fun = fun;
    cr->args = NULL;
    cr->ret = NULL;

    cr->status = MR_READY;

    cr->stack_size = stack_size;
    cr->stack = mmap(NULL, cr->stack_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_STACK | MAP_GROWSDOWN | MAP_ANONYMOUS,
                    -1, 0);
    if (cr->stack == MAP_FAILED)
    {
        free(cr);
        return NULL;
    }

    return cr;
}

static void cr_fini(struct coroutine_t *cr)
{
    if (cr == NULL)
        return;
    
    munmap(cr->stack, cr->stack_size);
    free(cr);
}

struct mroutine_t *mr_ini(int coroutine_num, int stack_size)
{
    struct mroutine_t *mr = (struct mroutine_t *)malloc(sizeof (struct mroutine_t));
    if (mr == NULL)
        return NULL;

    if (stack_size <= 0)
        stack_size = DEF_STACK_SIZE;
    else if (stack_size < MIN_STACK_SIZE)
        stack_size = MIN_STACK_SIZE;
    mr->stack_size = stack_size;

    if (coroutine_num <= DEF_COROUTINE_NUM)
        coroutine_num = DEF_COROUTINE_NUM;
    mr->cr_num = 0;
    mr->cr_max = 2 * coroutine_num;
    mr->crs = (struct coroutine_t **)malloc(mr->cr_max * sizeof (struct coroutine_t *));
    if (mr->crs == NULL)
    {
        free(mr);
        return NULL;
    }

    memset(mr->crs, 0, mr->cr_max * sizeof (struct coroutine_t *));
    mr->cr_mid = -1;

    return mr;
}


void mr_fini(struct mroutine_t *mr)
{
    if (mr == NULL)
        return;

    int check_num;
    int i;
    for (i = 0, check_num = 0; i < mr->cr_max && check_num < mr->cr_num; i++)
    {
        if (mr->crs[i] != NULL)
        {
            check_num++;
            cr_fini(mr->crs[i]);
            mr->crs[i] = NULL;
        }
    }
    free(mr->crs);
    mr->crs = NULL;
}

mid_t mr_create(struct mroutine_t *mr, mr_fun_t fun)
{
    struct coroutine_t *cr = cr_ini(fun, mr->stack_size);
    if (cr == NULL)
        return -1;

    if (mr->cr_num >= mr->cr_max)
    {
        mid_t mid = mr->cr_max;
        struct coroutine_t **temp = (struct coroutine_t **)realloc(mr->crs, 2 * mr->cr_max * sizeof (struct coroutine_t *)); 
        if (temp == NULL)
            return -1;

        mr->crs = temp;
        mr->cr_max = 2 * mr->cr_max;
        mr->cr_num++;

        mr->crs[mid] = cr;
        return mid;
    }
    else
    {
        int check_num;
        int i;
        for (i = 0, check_num = 0; i < mr->cr_max && check_num <= mr->cr_num; i++)
        {
            if (mr->crs[i] == NULL)
            {
                mr->cr_num++;
                mr->crs[i] = cr;
                return i;
            }
        }
    }

    return -1;
}

static void fun_wrap(uint32_t low, uint32_t high)
{
    uintptr_t ptr = ((uintptr_t)high) << 32 | (uintptr_t)low;
    struct mroutine_t *mr = (struct mroutine_t *)ptr;
    struct coroutine_t *cr = mr->crs[mr->cr_mid];

    cr->ret = cr->fun(mr, cr->args);

    cr->status = MR_DEAD;
}

void *mr_resume(struct mroutine_t *mr, mid_t mid, void *args)
{
    if (mr == NULL || mid < 0 || mid >= mr->cr_max || mr->crs[mid] == NULL)
        return NULL;

    struct coroutine_t *cr = mr->crs[mid];
    void *ret = NULL;
    switch (cr->status)
    {
    case MR_READY:
        getcontext(&cr->cur);
        cr->cur.uc_stack.ss_sp = cr->stack;
        cr->cur.uc_stack.ss_size = cr->stack_size;
        cr->cur.uc_link = &cr->prev;

        cr->args = args;
        cr->status = MR_RUNNING;

        cr->prev_mid = mr->cr_mid;
        mr->cr_mid = mid;

        uintptr_t ptr= (uintptr_t)mr;
        makecontext(&cr->cur, (void (*)(void))fun_wrap, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&cr->prev, &cr->cur);

        ret = cr->ret;
        mr->cr_mid = cr->prev_mid;
        
        if (cr->status == MR_DEAD)
        {
            cr_fini(cr);
            mr->crs[mid] = NULL;
            mr->cr_num--;
        }
        break;

    case MR_SUSPEND:
        cr->args = args;
        cr->status = MR_RUNNING;

        cr->prev_mid = mr->cr_mid;
        mr->cr_mid = mid;

        swapcontext(&cr->prev, &cr->cur);

        ret = cr->ret;
        mr->cr_mid = cr->prev_mid;
        break;
    }

    return ret;
}

void *mr_yield(struct mroutine_t *mr, void *ret)
{
    if (mr == NULL || mr->cr_mid == -1)
        return NULL;

    struct coroutine_t *cr = mr->crs[mr->cr_mid];
    cr->status = MR_SUSPEND;
    cr->ret = ret;
    mr->cr_mid = cr->prev_mid;
    swapcontext(&cr->cur, &cr->prev);

    return cr->args;
}

int mr_status(struct mroutine_t *mr, mid_t mid)
{
    if (mid < 0 || mid >= mr->cr_num || mr->crs[mid] == NULL)
        return MR_DEAD;
    else
        return mr->crs[mid]->status;
}

