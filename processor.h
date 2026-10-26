#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "stack.h"

#define COMMAND_QUANT 19
#define REGISTER_QUANT 4
#define CALL_DEPTH 64

enum command_code
{
    PUSH   = 0,
    ADD    = 1,
    SUB    = 2,
    DIV    = 3,
    OUT    = 4,
    HLT    = 5,
    POP    = 6,
    JMP    = 7,
    PUSH_R = 8,
    JA     = 9,
    JAE    = 10,
    JB     = 11,
    JBE    = 12,
    JE     = 13,
    JNE    = 14,
    CALL   = 15,
    RET    = 16,
    MUL    = 17,
    DEL    = 18
};

enum register_name
{
    REGBEGIN = 20,
    A_r = 20,
    B_r = 21,
    C_r = 22,
    D_r = 23
};


struct Processor_s
{
    Stack_s stk;
    int pc;
    int halted;
    int (*command_lst[COMMAND_QUANT])(Processor_s* proc);
    StackEl value;
    StackEl regs[REGISTER_QUANT];
    int ret_stack[CALL_DEPTH];
    int ret_ptr;
};


int  ProcessorCreate  (Processor_s* proc, size_t stack_capacity);
void ProcessorDestroy (Processor_s* proc);

int Push   (Processor_s* proc);
int Add    (Processor_s* proc);
int Sub    (Processor_s* proc);
int Mul    (Processor_s* proc);
int Div    (Processor_s* proc);
int Out    (Processor_s* proc);
int Hlt    (Processor_s* proc);
int Pop    (Processor_s* proc);

int Jmp    (Processor_s* proc);
int Ja     (Processor_s* proc);
int Jae    (Processor_s* proc);
int Jb     (Processor_s* proc);
int Jbe    (Processor_s* proc);
int Je     (Processor_s* proc);
int Jne    (Processor_s* proc);

int Call (Processor_s* proc);
int Ret  (Processor_s* proc);

int Push_r (Processor_s* proc);

int Del (Processor_s* proc);


int*   ReadFile (const char* file_name);
size_t FileSize (const char* file_name);

int DoCommand(Processor_s* proc, int* command);


#ifdef STACK_DEBUG
extern FILE* err_list;

enum proc_errs
{
    PROC_NO_ERR   = 0,
    PROC_NULL_ERR = 1,
    PC_ERR        = 2,
    CMD_ERR       = 3
};

const char* ProcErrorName(int err);

void ProcDump(const Processor_s* proc);

int ProcOK(const Processor_s* proc, const int* command);
#endif //STACK_DEBUG

#endif //PROCESSOR_H
