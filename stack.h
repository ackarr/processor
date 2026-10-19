#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef double StackEl;

#define STACK_DEBUG

#ifdef STACK_DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif //STACK_DEBUG

const StackEl POISON = NAN;

struct Stack_s
{
    StackEl* data;
    size_t size;
    size_t capacity;
    int err;

    ON_DBG(const char* stk_name; const void* stk_adr;)
};

typedef struct Stack_s Stack_s;

enum errs
{
    NO_ERR = 0,
    CAPACITY_ERR,
    DATA_SIZE_ERR,
    CAPACITY_SIZE_ERR,
    DATA_INIT_ERR,
    NULL_PTR_ERR,
    ALLOC_ERR,
    EMPTY_STACK_ERR,
    STK_ADR_ERR
};


int StackCreate_(Stack_s* stk, const size_t stack_size
                 ON_DBG(, const char* stk_name, const char* file, int line));

int StackOK_(Stack_s* stk
             ON_DBG(, const char* stk_name, const char* file, int line));

int StackPush_(Stack_s* stk, StackEl value
               ON_DBG(, const char* stk_name, const char* file, int line));

StackEl StackPop_(Stack_s* stk
                  ON_DBG(, const char* stk_name, const char* file, int line));

StackEl StackGet_(Stack_s* stk
                  ON_DBG(, const char* stk_name, const char* file, int line));

void StackPrint_(Stack_s* stk
                 ON_DBG(, const char* stk_name, const char* file, int line));

void StackClear_(Stack_s* stk
                 ON_DBG(, const char* stk_name, const char* file, int line));

extern const char* ErrorName(int err);

extern void StackLogErr_(const Stack_s* stk, int err
                         ON_DBG(, const char* stk_name, const char* file, int line));

ON_DBG(extern void StackDump_(const Stack_s* stk, const char* stk_name,
                              const char* file, int line);)


#define StackCreate(stk, size) \
        StackCreate_((stk), (size) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackOK(stk) \
        StackOK_((stk) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackPush(stk, value) \
        StackPush_((stk), (value) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackPop(stk) \
        StackPop_((stk) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackGet(stk) \
        StackGet_((stk) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackPrint(stk) \
        StackPrint_((stk) ON_DBG(, #stk, __FILE__, __LINE__))

#define StackClear(stk) \
        StackClear_((stk) ON_DBG(, #stk, __FILE__, __LINE__))

#endif //STACK_H
