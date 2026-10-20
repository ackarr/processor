#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_WORD 10
#define COM_COUNT 100

enum command_code
{
    PUSH = 0,
    ADD  = 1,
    SUB  = 2,
    DIV  = 3,
    OUT  = 4,
    HLT  = 5,
    POP  = 6
};

enum register_name
{
    REGBEGIN = 20,
    A_r = 20
};


int Decipher(FILE* data_d);


int main()
{
    FILE* asemb_d = fopen("assembler.txt", "r");

    if (!asemb_d)
    {
        perror("fopen assembler.txt");
        return 1;
    }

    Decipher(asemb_d);

    fclose(asemb_d);
}

int Decipher(FILE* asemb_d)
{
    const char* command_lst[] = {"PUSH", "ADD", "SUB", "DIV", "OUT", "HLT", "POP"};
    const char register_lst[] = {"A"};

    int* buf = (int*) calloc(COM_COUNT, sizeof(int));
    size_t ptr = 0;

    int c = 0, num = 0;
    char command[MAX_WORD] = {};

    char reg_name = 0;

    size_t capacity =  COM_COUNT;

    while((c = fscanf(asemb_d, "%9s", command)) == 1)
    {
        if(!strcmp(command_lst[PUSH], command))
        {
            buf[ptr] = PUSH;
            ptr++;
            c = fscanf(asemb_d, "%d", &num);
            buf[ptr] = num;
            ptr++;
        }

        else if(!strcmp(command_lst[ADD], command))
        {
            buf[ptr] = ADD;
            ptr++;
        }

        else if(!strcmp(command_lst[SUB], command))
        {
            buf[ptr] = SUB;
            ptr++;
        }

        else if(!strcmp(command_lst[DIV], command))
        {
            buf[ptr] = DIV;
            ptr++;
        }

        else if(!strcmp(command_lst[OUT], command))
        {
            buf[ptr] = OUT;
            ptr++;
        }

        else if(!strcmp(command_lst[HLT], command))
        {
            buf[ptr] = HLT;
            ptr++;
        }

        else if(!strcmp(command_lst[POP], command))
        {
            buf[ptr] = POP;
            ptr++;

            c = fscanf(asemb_d, " %c", &reg_name);

            if(reg_name == register_lst[A_r - REGBEGIN])
            {
                buf[ptr] = A_r;
                ptr++;
            }

            else printf("register");
        }

        if(capacity < ptr + 2)
        {
            int* temp = (int*) realloc(buf, capacity * 2 * sizeof(int));
            capacity *= 2;
            buf = temp;
        }
    }

    FILE* data = fopen("data.txt", "w");

    for(int i = 0; i < ptr; i++)
    {
        fprintf(data, "%d ", buf[i]);
    }

    fclose(data);

    free(buf);
}
