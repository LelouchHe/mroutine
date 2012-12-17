#ifndef _MCONTEXT_H
#define _MCONTEXT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MC_USE_ASM

#include <ucontext.h>

struct mcontext_t
{
    ucontext_t uc;
};

#else // MC_USE_ASM

#ifdef __x86_64__ // x86_64

// rip, rsp, rbp, rbx, r12, r13, r14, r15
struct mcontext_t
{
    void *regs[8];
    struct mcontext_t *link; // 结束后返回的context
};

#elif defined(__i386) || defined(__i386__) // x86

#ifdef __PIC__

// eip, esp, ebp, ebx
struct mcontext_t
{
    void *regs[4];
    struct mcontext_t *link; // 结束后返回的context
};

#else // PIC

// eip, esp, ebp
struct mcontext_t
{
    void *regs[3];
    struct mcontext_t *link; // 结束后返回的context
};

#endif // PIC

#endif // arch

#endif // MC_USE_ASM

void mc_get(struct mcontext_t *mc);
void mc_fill(struct mcontext_t *mc, char *stack, int stack_size, struct mcontext_t *mc_prev);
void mc_set(const struct mcontext_t *mc);
void mc_swap(struct mcontext_t *omc, struct mcontext_t *mc);

// used only for this
void mc_make(struct mcontext_t *mc, void (*fun)(), uint32_t low, uint32_t high);

#ifdef __cplusplus
}
#endif

#endif
