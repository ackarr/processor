#ifndef PROCESSOR_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "stack.h"

#define COMMAND_QUANT 6


enum command_code
{
    PUSH = 0,
    ADD  = 1,
    SUB  = 2,
    DIV  = 3,
    OUT  = 4,
    HLT  = 5
};


struct Processor_s
{
    Stack_s stk;
    int pc;
    int halted;
    int (*command_lst[COMMAND_QUANT])(Processor_s* proc);
    StackEl value;
};


int Push(Processor_s* proc);
int Add (Processor_s* proc);
int Sub (Processor_s* proc);
int Div (Processor_s* proc);
int Out (Processor_s* proc);
int Hlt (Processor_s* proc);

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
