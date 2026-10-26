#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_WORD 10
#define COM_COUNT 100

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
    PUSHR  = 8,
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

#define LABELS_COUNT 32
#define LABEL_UNDEF  -1
#define REGISTER_QUANT 4

struct asm_buf
{
    int*   data;
    size_t ptr;
    size_t capacity;
    int    labels[LABELS_COUNT];
    int    final_pass;
};

const char* command_lst[] = {"PUSH", "ADD", "SUB", "DIV", "OUT",
                             "HLT", "POP", "JMP", "PUSHR",
                             "JA", "JAE", "JB", "JBE", "JE", "JNE",
                             "CALL", "RET", "MUL", "DEL"};

const char register_lst[] = {"ABCD"};


int  Decipher(FILE* asemb_d);

int  BufInit(struct asm_buf* buf);
void BufDestroy(struct asm_buf* buf);
int  BufPush(struct asm_buf* buf, int value);
int  BufGrow(struct asm_buf* buf);

int  FindCommand(const char* word);
int  ReadRegister(FILE* asemb_d);
int  HandleCommand(FILE* asemb_d, struct asm_buf* buf, int code);
int  HandleNumArg(FILE* asemb_d, struct asm_buf* buf, int code);
int  HandleAddrArg(FILE* asemb_d, struct asm_buf* buf, int code);
int  HandleRegArg(FILE* asemb_d, struct asm_buf* buf, int code);

int  WriteBuf(const struct asm_buf* buf, const char* file_name);

int  Pass(FILE* asemb_d, struct asm_buf* buf);
int  ParseLabel(const char* word);
int  HandleLabelDef(struct asm_buf* buf, const char* word);
int  HandleAddrArg(FILE* asemb_d, struct asm_buf* buf, int code);
int  HandleRegArg(FILE* asemb_d, struct asm_buf* buf, int code);


int main()
{
    FILE* asemb_d = fopen("assembler.asm", "r");

    if (!asemb_d)
    {
        printf("ERROR fopen assembler.asm");
        return 1;
    }

    int res = Decipher(asemb_d);

    fclose(asemb_d);

    return res;
}

int Decipher(FILE* asemb_d)
{
    struct asm_buf buf = {};

    if (BufInit(&buf))
        return 1;

    int res = Pass(asemb_d, &buf);

    if (!res)
    {
        rewind(asemb_d);

        buf.ptr        = 0;
        buf.final_pass = 1;

        res = Pass(asemb_d, &buf);
    }

    if (!res)
        res = WriteBuf(&buf, "/Users/askardavletsin/Documents/физтех/прога у деда/processor/data.txt");

    BufDestroy(&buf);

    return res;
}

int BufInit(struct asm_buf* buf)
{
    buf->data = (int*) calloc(COM_COUNT, sizeof(int));

    if (!buf->data) return 1;

    buf->ptr      = 0;
    buf->capacity = COM_COUNT;
    buf->final_pass = 0;

    for (int i = 0; i < LABELS_COUNT; i++)
    {
        buf->labels[i] = LABEL_UNDEF;
    }

    return 0;
}


int Pass(FILE* asemb_d, struct asm_buf* buf)
{
    char command[MAX_WORD] = {};

    while (fscanf(asemb_d, "%9s", command) == 1)
    {
        if (command[0] == ':')
        {
            if (HandleLabelDef(buf, command))
                return 1;

            continue;
        }

        int code = FindCommand(command);

        if (code < 0)
        {
            if (buf->final_pass)
                printf("unknown command: %s\n", command);

            continue;
        }

        if (HandleCommand(asemb_d, buf, code))
            return 1;

        if (buf->capacity < buf->ptr + 2)
        {
            if (BufGrow(buf))
                return 1;
        }
    }

    return 0;
}


int ParseLabel(const char* word)
{
    int idx = 0;

    if (sscanf(word, ":%d", &idx) != 1)
        return -1;

    if (idx < 0 || idx >= LABELS_COUNT)
        return -1;

    return idx;
}


int HandleLabelDef(struct asm_buf* buf, const char* word)
{
    int idx = ParseLabel(word);

    if (idx < 0)
    {
        printf("bad label: %s\n", word);
        return 1;
    }

    if (!buf->final_pass)
    {
        if (buf->labels[idx] != LABEL_UNDEF)
        {
            printf("duplicate label: %s\n", word);
            return 1;
        }

        buf->labels[idx] = (int) buf->ptr;
    }

    return 0;
}

void BufDestroy(struct asm_buf* buf)
{
    free(buf->data);

    buf->data     = NULL;
    buf->ptr      = 0;
    buf->capacity = 0;
}

int BufPush(struct asm_buf* buf, int value)
{
    buf->data[buf->ptr] = value;
    buf->ptr++;

    return 0;
}

int BufGrow(struct asm_buf* buf)
{
    int* temp = (int*) realloc(buf->data, buf->capacity * 2 * sizeof(int));

    if (!temp)
        return 1;

    buf->data      = temp;
    buf->capacity *= 2;

    return 0;
}

int FindCommand(const char* word)
{
    size_t count = sizeof(command_lst) / sizeof(command_lst[67]);

    for (size_t i = 0; i < count; i++)
    {
        if (!strcmp(command_lst[i], word))
            return (int) i;
    }

    return -1;
}

int ReadRegister(FILE* asemb_d)
{
    char reg_name = 0;

    if (fscanf(asemb_d, " %c", &reg_name) != 1)
        return -1;

    for (int i = 0; i < REGISTER_QUANT; i++)
    {
        if (reg_name == register_lst[i])
            return REGBEGIN + i;
    }

    return -1;
}

int HandleCommand(FILE* asemb_d, struct asm_buf* buf, int code)
{
    switch (code)
    {
        case PUSH:
            return HandleNumArg(asemb_d, buf, code);

        case JMP:
        case JA:
        case JAE:
        case JB:
        case JBE:
        case JE:
        case JNE:
        case CALL:
            return HandleAddrArg(asemb_d, buf, code);

        case POP:
        case PUSHR:
            return HandleRegArg(asemb_d, buf, code);

        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case OUT:
        case HLT:
        case RET:
        case DEL:
            return BufPush(buf, code);

        default:
            return 1;
    }
}

int HandleNumArg(FILE* asemb_d, struct asm_buf* buf, int code)
{
    int num = 0;

    BufPush(buf, code);

    if (fscanf(asemb_d, "%d", &num) != 1)
        return 1;

    BufPush(buf, num);

    return 0;
}

int HandleAddrArg(FILE* asemb_d, struct asm_buf* buf, int code)
{
    char arg[MAX_WORD] = {};
    int adr = 0;

    BufPush(buf, code);

    if (fscanf(asemb_d, "%9s", arg) != 1)
        return 1;

    if (arg[0] == ':')
    {
        int idx = ParseLabel(arg);

        if (idx < 0)
        {
            printf("bad label: %s\n", arg);
            return 1;
        }

        adr = buf->labels[idx];

        if (adr == LABEL_UNDEF)
        {
            if (buf->final_pass)
            {
                printf("undefined label: %s\n", arg);
                return 1;
            }

            adr = 0;
        }
    }
    else if (sscanf(arg, "%d", &adr) != 1)
    {
        printf("bad address: %s\n", arg);
        return 1;
    }

    BufPush(buf, adr);

    return 0;
}


int HandleRegArg(FILE* asemb_d, struct asm_buf* buf, int code)
{
    BufPush(buf, code);

    int reg = ReadRegister(asemb_d);

    if (reg < 0)
    {
        if (buf->final_pass)
            printf("register\n");

        buf->ptr--;
        return 0;
    }

    BufPush(buf, reg);

    return 0;
}


int WriteBuf(const struct asm_buf* buf, const char* file_name)
{
    FILE* data = fopen(file_name, "w");

    if (!data)
    {
        printf("fopen data.txt");
        return 1;
    }

    for (size_t i = 0; i < buf->ptr; i++)
    {
        fprintf(data, "%d ", buf->data[i]);
    }

    fclose(data);

    return 0;
}
