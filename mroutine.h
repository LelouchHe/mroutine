#ifndef _MROUTINE_H
#define _MROUTINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define MRT_DEAD 0
#define MRT_READY 1
#define MRT_RUNNING 2
#define MRT_SUSPEND 3

struct mroutine_t;
typedef int mid_t;

struct mroutine_t *mrt_ini();
void mrt_fini(struct mroutine_t *mrt);

// >=0: 正常
// -1 : 错误
mid_t mrt_create(struct mroutine_t *mrt, void *(*fun)(void *), void *args);

// resume的返回值是yield的ret
// 目前维持这样的接口,否则routine之间只能通过全局变量通信
// 那样太恶心了
void *mrt_resume(struct mroutine_t *mrt, mid_t mid);
int mrt_yield(struct mroutine_t *mrt, void *ret);

int mrt_status(struct mroutine_t *mrt, mid_t mid);

#ifdef __cplusplus
}
#endif

#endif
