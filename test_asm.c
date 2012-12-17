#include <stdint.h>

#define REG_IP 0
#define REG_SP 1
#define REG_BP 2
#define REG_BX 3
#define REG_DI 4
#define REG_SI 5
#define REG_NUM 6

// rip, rsp, rbp, rbx, rdi, rsi
typedef void *ctx[REG_NUM];

struct mcontext_t
{
    ctx regs;
};

void mc_make_stack(struct mcontext_t *mc, char *stack, int stack_size)
{
    mc->regs[REG_SP] = (void *)(stack + stack_size);
    mc->regs[REG_BP] = mc->regs[REG_SP];
    mc->regs[REG_BX] = (void *)0;
}

void mc_make_entry(struct mcontext_t *mc, void (*fun)(), int low, int high)
{
    mc->regs[REG_IP] = (void *)fun;
    mc->regs[REG_DI] = (void *)low;
    mc->regs[REG_SI] = (void *)high;
}

void mc_set(void *regs[REG_NUM])
{
    __asm__ __volatile__ (
    "movq 24(%0), %%rbx\n\t"
    "movq 16(%0), %%rbp\n\t"
    "movq 8(%0), %%rsp\n\t"
    "movq (%0), %%rax\n\t"
    "movq %%rax, 8(%%rsp)\n\t"
    :
    : "D"(regs));
}

void magic_jmp()
{
    __asm__ __volatile__(
    "movq 16(%rsp), %rdi\n\t"
    "jmp mc_set\n\t");
}

void mc_make_link(struct mcontext_t *now, struct mcontext_t *prev)
{
    uint64_t *stack = now->regs[REG_SP];
    stack[-1] = (uint64_t)prev;
    stack[-2] = (uint64_t)magic_jmp;
    now->regs[REG_SP] = (void *)(&stack[-2]);
}

void mc_swap(void *from[REG_NUM], void *to[REG_NUM])
{
    __asm__ __volatile__ (
    "leaq 1f(%%rip), %%rax\n\t"
    "movq %%rax, (%0)\n\t" 
    "movq %%rsp, 8(%0)\n\t" 
    "movq %%rbp, 16(%0)\n\t"
    "movq %%rbx, 24(%0)\n\t" 
    "movq 24(%1), %%rbx\n\t"
    "movq 16(%1), %%rbp\n\t"
    "movq 8(%1), %%rsp\n\t"
    "jmpq *(%1)\n"
    "1:\n"
    : "=D"(from)
    : "S"(to)
    : "%rax", "memory");
}

//  test

#include <stdio.h>

char stack[1024];
struct mcontext_t now, prev;

void test_fun(int low, int high)
{
    printf("in test_fun: %d %d\n", low, high);
    mc_set(prev.regs);
}

int main()
{

    mc_make_stack(&now, stack, sizeof (stack));
    mc_make_entry(&now, test_fun, 0, 100);
    mc_make_link(&now, &prev);
    
    printf("before swap\n");

    mc_swap(prev.regs, now.regs);

    printf("after swap\n");

    return 0;
}











