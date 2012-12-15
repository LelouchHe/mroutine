#ifndef _MROUTINE_H
#define _MROUTINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define MR_DEAD 0
#define MR_READY 1
#define MR_RUNNING 2
#define MR_SUSPEND 3

struct mroutine_t;
typedef int mid_t;
typedef void *(*mr_fun_t)(struct mroutine_t *, void *);

struct mroutine_t *mr_ini(int coroutine_num, int stack_size);
void mr_fini(struct mroutine_t *mr);

// >=0: 正常
// -1 : 错误
mid_t mr_create(struct mroutine_t *mr, mr_fun_t fun);

// 如果没有返回结果或者结束了,都返回NULL
// 需要通过status接口判断具体状态
void *mr_resume(struct mroutine_t *mr, mid_t mid, void *args);
void *mr_yield(struct mroutine_t *mr, void *ret);

int mr_status(struct mroutine_t *mr, mid_t mid);

#ifdef __cplusplus
}
#endif

#endif
