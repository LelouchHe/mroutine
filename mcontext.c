#include "mcontext.h"

#ifndef MC_USE_ASM

#include <stdint.h>

void mc_get(struct mcontext_t *mc)
{
    getcontext(&mc->uc);
}

void mc_fill(struct mcontext_t *mc, char *stack, int stack_size, struct mcontext_t *mc_prev)
{
    mc->uc.uc_stack.ss_sp = stack;
    mc->uc.uc_stack.ss_size = stack_size;
    mc->uc.uc_link = &mc_prev->uc;
}

void mc_set(const struct mcontext_t *mc)
{
    setcontext(&mc->uc);
}

void mc_swap(struct mcontext_t *omc, struct mcontext_t *mc)
{
    swapcontext(&omc->uc, &mc->uc);
}

void mc_make(struct mcontext_t *mc, void (*fun)(), uint32_t low, uint32_t high)
{
    makecontext(&mc->uc, fun, 2, low, high);
}

#else // 使用汇编必须显式定义MC_USE_ASM,因为这个只能支持两个平台

#ifdef __x86_64__

static void mc_wrap_main(void)
{
  __asm__ __volatile__ ("\tmovq %r13, %rdi\n\tjmpq *%r12\n");
}

typedef void *mc_regs[8];  /* rip, rsp, rbp, rbx, r12, r13, r14, r15 */
static inline void mc_swap_asm(mc_regs from, mc_regs to)
{
  __asm__ __volatile__ (
    "leaq 1f(%%rip), %%rax\n\t"
    "movq %%rax, (%0)\n\t" 
    "movq %%rsp, 8(%0)\n\t" 
    "movq %%rbp, 16(%0)\n\t"
    "movq %%rbx, 24(%0)\n\t" 
    "movq %%r12, 32(%0)\n\t" 
    "movq %%r13, 40(%0)\n\t"
    "movq %%r14, 48(%0)\n\t" 
    "movq %%r15, 56(%0)\n\t"
    "movq 56(%1), %%r15\n\t" 
    "movq 48(%1), %%r14\n\t" 
    "movq 40(%1), %%r13\n\t"
    "movq 32(%1), %%r12\n\t" 
    "movq 24(%1), %%rbx\n\t"
    "movq 16(%1), %%rbp\n\t"
    "movq 8(%1), %%rsp\n\t"
    "jmpq *(%1)\n" "1:\n"
    : "+S" (from), "+D" (to) :
    : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11", "memory", "cc");
}

typedef void *mc_regs[8];  /* rip, rsp, rbp, rbx, r12, r13, r14, r15 */
#define MC_MAKE(regs, func, low, high) \
    regs[0] = (void *)(coco_wrap_main); \
    regs[1] = (void *)(stack); \
    regs[2] = (void *)0; \
    regs[3] = (void *)0; \
    regs[4] = (void *)(func); \
    regs[5] = (void *)((uintptr_t)low | (((uintptr_t)high) << 32)); \
    regs[6] = (void *)0; \
    regs[7] = (void *)0; \

#define MC_FILL(regs, stack, stack_size, prev_regs)
    regs[1] = (void *)(stack);
    stack[0] = prev_regs[0];

#elif defined(__i386) || defined(__i386__) // x86

#ifdef __PIC__
typedef void *mc_regs[4];  /* eip, esp, ebp, ebx */
static inline void mc_swap_asm(mc_regs from, mc_regs to) 
{
  __asm__ __volatile__ (
    "call 1f\n"
    "1:\tpopl %%eax\n\t"
    "addl $(2f-1b),%%eax\n\t"
    "movl %%eax, (%0)\n\t"
    "movl %%esp, 4(%0)\n\t"
    "movl %%ebp, 8(%0)\n\t"
    "movl %%ebx, 12(%0)\n\t"
    "movl 12(%1), %%ebx\n\t"
    "movl 8(%1), %%ebp\n\t"
    "movl 4(%1), %%esp\n\t"
    "jmp *(%1)\n" "2:\n"
    : "+S" (from), "+D" (to) : : "eax", "ecx", "edx", "memory", "cc");
}

#else // PIC

typedef void *mc_regs[3];  /* eip, esp, ebp */
static inline void mc_swap_asm(mc_regs from, mc_regs to) 
{
  __asm__ __volatile__ (
    "movl $1f, (%0)\n\t"     // ip设为标号1,即跳过这段汇编码
    "movl %%esp, 4(%0)\n\t"
    "movl %%ebp, 8(%0)\n\t"
    "movl 8(%1), %%ebp\n\t"
    "movl 4(%1), %%esp\n\t"
    "jmp *(%1)\n"            // 跳转到对应ip处
    "1:\n"
    : "+S" (from), "+D" (to) : : "eax", "ebx", "ecx", "edx", "memory", "cc");
}

#endif // PIC

#define MC_MAKE(regs, func, stack, stack_size, low, high) \
    regs[0] = (void *)(func); \
    regs[1] = (void *)(stack); \
    regs[2] = (void *)0; \
    stack[0] = 0xdeadc0c0;  /* Dummy return address. */ \
    coco->arg0 = (size_t)(a0);

#define MC_FILL(regs, stack, stack_size, prev_regs)
    regs[1] = (void *)(stack);
    stack[0] = prev_regs[0];

#endif // arch

#endif // MC_USE_ASM


void mc_get(struct mcontext_t *mc)
{}

void mc_fill(struct mcontext_t *mc, char *stack, int stack_size, struct mcontext_t *mc_prev)
{}

void mc_set(const struct mcontext_t *mc)
{}

void mc_swap(struct mcontext_t *omc, struct mcontext_t *mc)
{
}


void mc_make(struct mcontext_t *mc, void (*fun)(), uint32_t low, uint32_t high)
{}

#endif
