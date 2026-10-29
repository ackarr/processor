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
    proc->command_lst[MUL]    = Mul;
    proc->command_lst[DIV]    = Div;
    proc->command_lst[OUT]    = Out;
    proc->command_lst[HLT]    = Hlt;
    proc->command_lst[POP]    = Pop;
    proc->command_lst[JMP]    = Jmp;
    proc->command_lst[PUSH_R] = Push_r;

    proc->command_lst[JA]     = Ja;
    proc->command_lst[JAE]    = Jae;
    proc->command_lst[JB]     = Jb;
    proc->command_lst[JBE]    = Jbe;
    proc->command_lst[JE]     = Je;
    proc->command_lst[JNE]    = Jne;

    proc->command_lst[CALL]   = Call;
    proc->command_lst[RET]    = Ret;

    proc->command_lst[DEL]   = Del;

    proc->ret_ptr = 0;

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


typedef StackEl (*BinaryOp)(StackEl left, StackEl right);

static int ApplyBinaryOp(Processor_s* proc, BinaryOp op)
{
    StackEl right = StackPop(&proc->stk);
    StackEl left  = StackPop(&proc->stk);

    StackPush(&proc->stk, op(left, right));

    proc->pc += 1;
    return 0;
}

static StackEl OpAdd(StackEl a, StackEl b) { return a + b; }
static StackEl OpSub(StackEl a, StackEl b) { return a - b; }
static StackEl OpMul(StackEl a, StackEl b) { return a * b; }
static StackEl OpDiv(StackEl a, StackEl b) { return a / b; }

int Add(Processor_s* proc) { return ApplyBinaryOp(proc, OpAdd); }
int Sub(Processor_s* proc) { return ApplyBinaryOp(proc, OpSub); }
int Mul(Processor_s* proc) { return ApplyBinaryOp(proc, OpMul); }
int Div(Processor_s* proc) { return ApplyBinaryOp(proc, OpDiv); }


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
    int idx = (int) proc->value - REGBEGIN;

    StackEl val = StackPop(&(proc->stk));

    proc->regs[idx] = val;

    proc->pc += 2;

    return 0;
}


int Jmp(Processor_s* proc)
{
    proc->pc = (int) proc->value;

    return 0;
}


int JmpIf(Processor_s* proc, int cond)
{
    if (cond)
        return Jmp(proc);

    proc->pc += 2;

    return 0;
}

typedef bool (*BinaryPred)(StackEl left, StackEl right);

static int ApplyJumpPred(Processor_s* proc, BinaryPred pred)
{
    StackEl right = StackPop(&proc->stk);
    StackEl left  = StackPop(&proc->stk);

    return JmpIf(proc, pred(left, right));
}

static bool PredGt(StackEl a, StackEl b) { return a >  b; }
static bool PredGe(StackEl a, StackEl b) { return a >= b; }
static bool PredLt(StackEl a, StackEl b) { return a <  b; }
static bool PredLe(StackEl a, StackEl b) { return a <= b; }
static bool PredEq(StackEl a, StackEl b) { return a == b; }
static bool PredNe(StackEl a, StackEl b) { return a != b; }

int Ja (Processor_s* proc) { return ApplyJumpPred(proc, PredGt); }
int Jae(Processor_s* proc) { return ApplyJumpPred(proc, PredGe); }
int Jb (Processor_s* proc) { return ApplyJumpPred(proc, PredLt); }
int Jbe(Processor_s* proc) { return ApplyJumpPred(proc, PredLe); }
int Je (Processor_s* proc) { return ApplyJumpPred(proc, PredEq); }
int Jne(Processor_s* proc) { return ApplyJumpPred(proc, PredNe); }




int Call(Processor_s* proc)
{
    ON_DBG(
    if (proc->ret_ptr >= CALL_DEPTH)
    {
        printf("call stack overflow\n");
        proc->halted = 1;
        return 1;
    })

    proc->ret_stack[proc->ret_ptr] = proc->pc + 2;
    proc->ret_ptr++;

    return Jmp(proc);
}

int Ret(Processor_s* proc)
{
    ON_DBG(
    if (proc->ret_ptr <= 0)
    {
        printf("RET without CALL\n");
        proc->halted = 1;
        return 1;
    })

    proc->ret_ptr--;
    proc->pc = proc->ret_stack[proc->ret_ptr];

    return 0;
}


int Push_r(Processor_s* proc)
{
    register_name name_r = (register_name) proc->value;

    StackPush(&(proc->stk), proc->regs[name_r - REGBEGIN]);

    proc->pc += 2;

    return 0;
}


int Del(Processor_s* proc)
{
    StackPop(&(proc->stk));

    proc->pc += 1;

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
