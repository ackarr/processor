#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


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

struct source
{
    char*  data;
    size_t size;
    size_t pos;
};

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

#define COMMANDS_COUNT (sizeof(command_lst) / sizeof(command_lst[0]))

unsigned long command_hash[COMMANDS_COUNT] = {};


int  SourceLoad(struct source* src, const char* file_name);
void SourceDestroy(struct source* src);
void StripComments(struct source* src);

int  Decipher(struct source* src);

int  BufInit   (struct asm_buf* buf);
void BufDestroy(struct asm_buf* buf);
int  BufPush   (struct asm_buf* buf, int value);
int  BufGrow   (struct asm_buf* buf);

unsigned long Djb2(const char* word);
void InitCommandHashes();

int  FindCommand    (const char* word);
int  ReadRegister   (struct source* src);
int  EmitCommand    (struct source* src, struct asm_buf* buf, int code);
int  EmitNumCommand (struct source* src, struct asm_buf* buf, int code);
int  EmitJumpCommand(struct source* src, struct asm_buf* buf, int code);
int  EmitRegCommand (struct source* src, struct asm_buf* buf, int code);

int  WriteBuf(const struct asm_buf* buf, const char* file_name);

int  AssemblePass(struct source* src, struct asm_buf* buf);
int  ParseLabel  (const char* word);
int  DefineLabel (struct asm_buf* buf, const char* word);


int main()
{
    struct source src = {};

    InitCommandHashes();

    if (SourceLoad(&src, "assembler.asm"))
    {
        printf("ERROR open assembler.asm\n");
        return 1;
    }

    int res = Decipher(&src);

    SourceDestroy(&src);

    return res;
}

int SourceLoad(struct source* src, const char* file_name)
{
    int fd = open(file_name, O_RDONLY);

    if (fd < 0)
        return 1;

    struct stat st = {};

    if (fstat(fd, &st) != 0 || st.st_size < 0)
    {
        close(fd);
        return 1;
    }

    size_t size = (size_t) st.st_size;

    char* data = (char*) calloc(size + 1, sizeof(char));

    if (!data)
    {
        close(fd);
        return 1;
    }

    ssize_t got = read(fd, data, size);

    close(fd);

    if (got < 0)
    {
        free(data);
        return 1;
    }

    data[got] = '\0';

    src->data = data;
    src->size = (size_t) got;
    src->pos  = 0;

    return 0;
}

void SourceDestroy(struct source* src)
{
    free(src->data);

    src->data = NULL;
    src->size = 0;
    src->pos  = 0;
}

void StripComments(struct source* src)
{
    for (size_t i = 0; i < src->size; i++)
    {
        int line_start = (i == 0) || (src->data[i - 1] == '\n');

        if (!line_start || src->data[i] != ';')
            continue;

        while (i < src->size && src->data[i] != '\n')
            src->data[i++] = ' ';
    }
}

int Decipher(struct source* src)
{
    struct asm_buf buf = {};

    StripComments(src);

    if (BufInit(&buf))
        return 1;

    int res = AssemblePass(src, &buf);

    if (!res)
    {
        src->pos = 0;

        buf.ptr        = 0;
        buf.final_pass = 1;

        res = AssemblePass(src, &buf);
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

    buf->ptr        = 0;
    buf->capacity   = COM_COUNT;
    buf->final_pass = 0;

    for (int i = 0; i < LABELS_COUNT; i++)
    {
        buf->labels[i] = LABEL_UNDEF;
    }

    return 0;
}


int AssemblePass(struct source* src, struct asm_buf* buf)
{
    char command[MAX_WORD] = {};
    int  read_len = 0;

    while (sscanf(src->data + src->pos, "%9s%n", command, &read_len) == 1)
    {
        src->pos += (size_t) read_len;

        if (command[0] == ':')
        {
            if (DefineLabel(buf, command))
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

        if (EmitCommand(src, buf, code))
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


int DefineLabel(struct asm_buf* buf, const char* word)
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

unsigned long Djb2(const char* word)
{
    unsigned long hash = 5381;

    for (; *word; word++)
        hash = ((hash << 5) + hash) + (unsigned char) *word;

    return hash;
}

void InitCommandHashes()
{
    for (size_t i = 0; i < COMMANDS_COUNT; i++)
        command_hash[i] = Djb2(command_lst[i]);
}

int FindCommand(const char* word)
{
    unsigned long hash = Djb2(word);

    for (size_t i = 0; i < COMMANDS_COUNT; i++)
    {
        if (command_hash[i] == hash && !strcmp(command_lst[i], word))
            return (int) i;
    }

    return -1;
}

int ReadRegister(struct source* src)
{
    char reg_name = 0;
    int  read_len = 0;

    if (sscanf(src->data + src->pos, " %c%n", &reg_name, &read_len) != 1)
        return -1;

    src->pos += (size_t) read_len;

    for (int i = 0; i < REGISTER_QUANT; i++)
    {
        if (reg_name == register_lst[i])
            return REGBEGIN + i;
    }

    return -1;
}

int EmitCommand(struct source* src, struct asm_buf* buf, int code)
{
    switch (code)
    {
        case PUSH:
            return EmitNumCommand(src, buf, code);

        case JMP:
        case JA:
        case JAE:
        case JB:
        case JBE:
        case JE:
        case JNE:
        case CALL:
            return EmitJumpCommand(src, buf, code);

        case POP:
        case PUSHR:
            return EmitRegCommand(src, buf, code);

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

int EmitNumCommand(struct source* src, struct asm_buf* buf, int code)
{
    int num      = 0;
    int read_len = 0;

    BufPush(buf, code);

    if (sscanf(src->data + src->pos, "%d%n", &num, &read_len) != 1)
        return 1;

    src->pos += (size_t) read_len;

    BufPush(buf, num);

    return 0;
}

int EmitJumpCommand(struct source* src, struct asm_buf* buf, int code)
{
    char arg[MAX_WORD] = {};
    int  adr      = 0;
    int  read_len = 0;

    BufPush(buf, code);

    if (sscanf(src->data + src->pos, "%9s%n", arg, &read_len) != 1)
        return 1;

    src->pos += (size_t) read_len;

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


int EmitRegCommand(struct source* src, struct asm_buf* buf, int code)
{
    BufPush(buf, code);

    int reg = ReadRegister(src);

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
