#include "processor.h"

int main()
{
    int* command = ReadFile("data.txt");

    Processor_s proc = {.pc = 0,
                        .halted = 0,
                        .command_lst = {Push, Add, Sub, Div, Out, Hlt, Pop},
                        .value = POISON};

    StackCreate(&proc.stk, 16);

    DoCommand(&proc, command);

    ON_DBG(ProcDump(&proc);)

    free(command);

    return 0;
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
    StackEl subtr = StackPop(&(proc->stk));
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
    register_lst[A_r] = A;

    proc->pc += 2;
}


int* ReadFile(const char* file_name)
{
    size_t file_size = FileSize(file_name) / 2 + 1;

    FILE* fd = fopen(file_name, "r");

    int* buf = (int*) calloc(file_size + 1, sizeof(int));

    int i = 0;
    for(int c = 0; (i < file_size) && fscanf(fd, "%d", &c) != EOF; i++)
    {
        buf[i] = c;
    }

    buf[i] = POISON;

    fclose(fd);

    return buf;
}

size_t FileSize (const char* file_name)
{
    struct stat st;

    stat(file_name, &st);

    return (size_t) st.st_size;
}


int DoCommand(Processor_s* proc, int* command)
{
    while((!proc->halted) && (command[proc->pc] != POISON))
    {
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
