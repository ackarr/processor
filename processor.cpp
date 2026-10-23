#include "processor.h"

int main()
{
    int* command = ReadFile("data.txt");

    if (!command)
    {
        printf("ReadFile data.txt");
        return 1;
    }

    Processor_s proc = {};

    if (ProcessorCreate(&proc, 16))
    {
        free(command);
        return 1;
    }

    DoCommand(&proc, command);

    ON_DBG(ProcDump(&proc);)

    printf("%lg", proc.regs[A_r - REGBEGIN]);

    ProcessorDestroy(&proc);

    free(command);

    return 0;
}


int ProcessorCreate(Processor_s* proc, size_t stack_capacity)
{
    if (!proc)
        return 1;

    proc->pc     = 0;
    proc->halted = 0;
    proc->value  = POISON;

    proc->command_lst[PUSH]   = Push;
    proc->command_lst[ADD]    = Add;
    proc->command_lst[SUB]    = Sub;
    proc->command_lst[DIV]    = Div;
    proc->command_lst[OUT]    = Out;
    proc->command_lst[HLT]    = Hlt;
    proc->command_lst[POP]    = Pop;
    proc->command_lst[JMP]    = Jmp;
    proc->command_lst[PUSH_R] = Push_r;

    for (int i = 0; i < REGISTER_QUANT; i++)
    {
        proc->regs[i] = 0;
    }

    StackCreate(&proc->stk, stack_capacity);

    return 0;
}

void ProcessorDestroy(Processor_s* proc)
{
    if (!proc)
        return;

    StackClear(&proc->stk);

    proc->pc     = 0;
    proc->halted = 1;
    proc->value  = POISON;
}


int Push(Processor_s* proc)
{
    StackPush(&(proc->stk), proc->value);

    proc->pc += 2;

    return 0;
}


int Add(Processor_s* proc)
{
    StackEl num1 = StackPop(&(proc->stk));
    StackEl num2 = StackPop(&(proc->stk));

    StackEl sum = num1 + num2;

    StackPush(&(proc->stk), sum);

    proc->pc += 1;

    return 0;
}


int Sub(Processor_s* proc)
{
    StackEl subtr   = StackPop(&(proc->stk));
    StackEl minuend = StackPop(&(proc->stk));

    StackEl diff = minuend - subtr;

    StackPush(&(proc->stk), diff);

    proc->pc += 1;

    return 0;
}


int Div(Processor_s* proc)
{
    StackEl divider  = StackPop(&(proc->stk));
    StackEl dividend = StackPop(&(proc->stk));

    StackEl priv = dividend / divider;

    StackPush(&(proc->stk), priv);

    proc->pc += 1;

    return 0;
}


int Out(Processor_s* proc)
{
    StackPrint(&(proc->stk));

    proc->pc += 1;

    return 0;
}


int Hlt(Processor_s* proc)
{
    StackClear(&(proc->stk));

    proc->halted = 1;

    return 0;
}


int Pop(Processor_s* proc)
{
    StackEl A = StackPop(&(proc->stk));

    proc->regs[A_r - REGBEGIN] = A;

    proc->pc += 2;

    return 0;
}


int Jmp(Processor_s* proc)
{
    proc->pc = (int) proc->value;

    return 0;
}


int Push_r(Processor_s* proc)
{
    register_name name_r = (register_name) proc->value;

    StackPush(&(proc->stk), proc->regs[name_r - REGBEGIN]);

    proc->pc += 2;

    return 0;
}


int* ReadFile(const char* file_name)
{
    size_t file_size = FileSize(file_name);

    if (file_size == 0)
        return NULL;

    size_t capacity = file_size / 2 + 1;

    FILE* fd = fopen(file_name, "r");

    if (!fd)
        return NULL;

    int* buf = (int*) calloc(capacity + 1, sizeof(int));

    if (!buf)
    {
        fclose(fd);
        return NULL;
    }

    size_t i = 0;
    int c    = 0;

    while (i < capacity && fscanf(fd, "%d", &c) == 1)
    {
        buf[i] = c;
        i++;
    }

    buf[i] = POISON;

    fclose(fd);

    return buf;
}

size_t FileSize(const char* file_name)
{
    struct stat st = {};

    if (stat(file_name, &st))
        return 0;

    return (size_t) st.st_size;
}


int DoCommand(Processor_s* proc, int* command)
{
    while ((!proc->halted) && (command[proc->pc] != POISON))
    {
        ON_DBG(ProcDump(proc);)

        ON_DBG
        (
            if (ProcOK(proc, command))
            {
                return 1;
            }
        )

        proc->value = command[proc->pc + 1];
        proc->command_lst[command[proc->pc]](proc);
    }

    return 0;
}


#ifdef STACK_DEBUG

int ProcOK(const Processor_s* proc, const int* command)
{
    FILE* out = err_list ? err_list : stderr;

    int err = PROC_NO_ERR;

    if (!proc || !command)
    {
        err = PROC_NULL_ERR;
    }
    else if (proc->pc < 0)
    {
        err = PC_ERR;
    }
    else if (command[proc->pc] < 0 || command[proc->pc] >= COMMAND_QUANT)
    {
        err = CMD_ERR;
    }

    if (err)
    {
        fprintf(out, "PROC ERROR %d (%s)\n", err, ProcErrorName(err));

        ProcDump(proc);
    }

    return err;
}

const char* ProcErrorName(int err)
{
    switch (err)
    {
        case PROC_NO_ERR:   return "no error";
        case PROC_NULL_ERR: return "processor pointer is NULL";
        case PC_ERR:        return "pc is out of range";
        case CMD_ERR:       return "unknown command code";
        default:            return "unknown error";
    }
}

void ProcDump(const Processor_s* proc)
{
    FILE* out = err_list ? err_list : stderr;

    fprintf(out, "Processor_s [%p]\n{\n", (const void*) proc);

    if (!proc)
    {
        fprintf(out, "}\n");
        return;
    }

    fprintf(out, "    pc       = %d\n", proc->pc);
    fprintf(out, "    halted   = %d\n", proc->halted);
    fprintf(out, "    value    = %lg\n", proc->value);
    fprintf(out, "}\n");

    StackDump_(&proc->stk, "proc->stk", __FILE__, __LINE__);
}

#endif //STACK_DEBUG
