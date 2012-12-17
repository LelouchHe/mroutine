#include <stdio.h>
#include <unistd.h>

// rip, rsp, rbp, rbx, r12, r13, r14, r15
typedef void *regs_t[4];

struct mcontext_t
{
    regs_t regs;
};

int mc_get(regs_t regs)
{
    __asm__ __volatile__ (
    "movq 8(%%rsp), %%rax\n\t"
    "movq %%rax, (%0)\n\t" 
    "movq %%rsp, 8(%0)\n\t" 
    "movq %%rbp, 16(%0)\n\t"
    "movq %%rbx, 24(%0)\n\t" 
    "movq $0, %%rax\n\t" 
    : "=D"(regs)
    :
    : "%rax", "memory");
}

int mc_set(regs_t regs)
{
    __asm__ __volatile__ (
    "movq 24(%0), %%rbx\n\t"
    "movq 16(%0), %%rbp\n\t"
    "movq 8(%0), %%rsp\n\t"
    "movq (%0), %%rax\n\t"
    "movq %%rax, 8(%%rsp)\n\t"
    "movq $1, %%rax\n\t" 
    :
    : "D"(regs));
}

void mc_swap(regs_t from, regs_t to)
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

struct mcontext_t mc1, mc2;

void test()
{
    printf("in test\n");
    mc_swap(mc2.regs, mc1.regs);
}

int main()
{
    if (mc_get(mc1.regs) == 0)
        printf("test\n");
    else 
        printf("magic\n");
    sleep(1);
    mc_set(mc1.regs);

    return 0;
}
