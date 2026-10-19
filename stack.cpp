#include "stack.h"

extern FILE* err_list = NULL;


extern const char* ErrorName(int err)
{
    switch (err)
    {
        case NO_ERR:            return "no error";
        case CAPACITY_ERR:      return "zero capacity";
        case DATA_SIZE_ERR:     return "data is NULL, but size != 0";
        case CAPACITY_SIZE_ERR: return "size is greater than capacity";
        case DATA_INIT_ERR:     return "data is not initialized";
        case NULL_PTR_ERR:      return "stack pointer is NULL";
        case ALLOC_ERR:         return "memory allocation failed";
        case EMPTY_STACK_ERR:   return "stack is empty";
        case STK_ADR_ERR:       return "stack was copied or corrupted";
        default:                return "unknown error";
    }
}


extern void StackLogErr_(const Stack_s* stk, int err
                         ON_DBG(, const char* stk_name, const char* file, int line))
{
    #ifdef STACK_DEBUG
    fprintf(err_list, "[%s:%d] ERROR %d (%s): %s @ %p\n",
            file, line, err, ErrorName(err), stk_name, (const void*) stk);

    StackDump_(stk, stk_name, file, line);
    #else
    (void) stk;
    fprintf(err_list, "ERROR %d (%s)\n", err, ErrorName(err));
    #endif //STACK_DEBUG
}


#ifdef STACK_DEBUG
extern void StackDump_(const Stack_s* stk, const char* stk_name,
                       const char* file, int line)
{
    const size_t DUMP_MAX = 100;


    fprintf(err_list, "Stack_s \"%s\" [%p] dumped at %s:%d\n{\n",
            stk_name, (const void*) stk, file, line);
    fprintf(err_list, "    size     = %zu\n", stk->size);
    fprintf(err_list, "    capacity = %zu\n", stk->capacity);
    fprintf(err_list, "    err      = %d\n", stk->err);
    fprintf(err_list, "    name     = %s\n", stk->stk_name ? stk->stk_name : "(null)");
    fprintf(err_list, "    adr      = %p\n", stk->stk_adr);
    fprintf(err_list, "    data [%p]", (const void*) stk->data);

    if (stk->data && stk->size <= stk->capacity)
    {
        fprintf(err_list, "\n    {\n");

        for (size_t i = 0; i < stk->capacity && i < DUMP_MAX; i++)
        {
            fprintf(err_list, "        %c[%zu] = %lg\n",
                    (i < stk->size) ? '*' : ' ', i, stk->data[i]);
        }

        fprintf(err_list, "    }\n");
    }
    else
    {
        fprintf(err_list, " (unavailable)\n");
    }

    fprintf(err_list, "}\n");
}
#endif //STACK_DEBUG


int StackCreate_(Stack_s* stk, const size_t stack_size
                 ON_DBG(, const char* stk_name, const char* file, int line))
{
    err_list = fopen("ERRORS.txt", "a");
    if (!err_list) err_list = stderr;


    if (!stk)
    {
        StackLogErr_(stk, NULL_PTR_ERR ON_DBG(, stk_name, file, line));
        return NULL_PTR_ERR;
    }

    stk->data = NULL;
    stk->size = 0;
    stk->capacity = 0;
    stk->err = NO_ERR;

    ON_DBG
    (
        stk->stk_name = stk_name;
        stk->stk_adr = stk;
    )

    if (stack_size == 0)
    {
        stk->err = CAPACITY_ERR;
        StackLogErr_(stk, CAPACITY_ERR ON_DBG(, stk_name, file, line));
        return CAPACITY_ERR;
    }

    stk->data = (StackEl*) calloc(stack_size, sizeof(stk->data[0]));
    if (!stk->data)
    {
        stk->err = ALLOC_ERR;
        StackLogErr_(stk, ALLOC_ERR ON_DBG(, stk_name, file, line));
        return ALLOC_ERR;
    }

    stk->capacity = stack_size;

    for (size_t i = 0; i < stk->capacity; i++)
    {
        stk->data[i] = POISON;
    }

    return NO_ERR;
}


int StackOK_(Stack_s* stk
             ON_DBG(, const char* stk_name, const char* file, int line))
{
    if (!stk)
    {
        StackLogErr_(stk, NULL_PTR_ERR ON_DBG(, stk_name, file, line));
        return NULL_PTR_ERR;
    }

    int err = NO_ERR;

    if (!stk->data)
    {
        err = stk->size ? DATA_SIZE_ERR : DATA_INIT_ERR;
    }
    else if (!stk->capacity)
    {
        err = CAPACITY_ERR;
    }
    else if (stk->size > stk->capacity)
    {
        err = CAPACITY_SIZE_ERR;
    }

    ON_DBG
    (
        if (!err && stk->stk_adr != stk)
        {
            err = STK_ADR_ERR;
        }
    )

    if (err)
    {
        stk->err = err;
        StackLogErr_(stk, err ON_DBG(, stk_name, file, line));
    }

    return err;
}


int StackPush_(Stack_s* stk, StackEl value
               ON_DBG(, const char* stk_name, const char* file, int line))
{
    int err = StackOK_(stk ON_DBG(, stk_name, file, line));
    if (err)
    {
        return err;
    }

    if (stk->size >= stk->capacity)
    {
        size_t new_capacity = stk->capacity * 3 / 2 + 1;

        StackEl* new_data = (StackEl*) realloc(stk->data,
                                               new_capacity * sizeof(StackEl));
        if (!new_data)
        {
            stk->err = ALLOC_ERR;
            StackLogErr_(stk, ALLOC_ERR ON_DBG(, stk_name, file, line));
            return ALLOC_ERR;
        }

        stk->data = new_data;

        for (size_t i = stk->capacity; i < new_capacity; i++)
        {
            stk->data[i] = POISON;
        }

        stk->capacity = new_capacity;
    }

    stk->data[stk->size++] = value;

    err = StackOK_(stk ON_DBG(, stk_name, file, line));
    if (err)
    {
        ON_DBG
        (
            fprintf(err_list, "[%s:%d] ERROR: StackPush(%s @ %p, %lg)\n",
                    file, line, stk_name, (const void*) stk, value);
        )
    }

    return err;
}


StackEl StackPop_(Stack_s* stk
                  ON_DBG(, const char* stk_name, const char* file, int line))
{
    if (StackOK_(stk ON_DBG(, stk_name, file, line)))
    {
        return POISON;
    }

    if (stk->size == 0)
    {
        stk->err = EMPTY_STACK_ERR;
        StackLogErr_(stk, EMPTY_STACK_ERR ON_DBG(, stk_name, file, line));
        return POISON;
    }

    StackEl output = stk->data[stk->size - 1];
    stk->data[stk->size - 1] = POISON;
    stk->size--;

    if (StackOK_(stk ON_DBG(, stk_name, file, line)))
    {
        return POISON;
    }

    return output;
}


StackEl StackGet_(Stack_s* stk
                  ON_DBG(, const char* stk_name, const char* file, int line))
{
    if (StackOK_(stk ON_DBG(, stk_name, file, line)))
    {
        return POISON;
    }

    if (stk->size == 0)
    {
        stk->err = EMPTY_STACK_ERR;
        StackLogErr_(stk, EMPTY_STACK_ERR ON_DBG(, stk_name, file, line));
        return POISON;
    }

    return stk->data[stk->size - 1];
}


void StackPrint_(Stack_s* stk
                 ON_DBG(, const char* stk_name, const char* file, int line))
{
    if (StackOK_(stk ON_DBG(, stk_name, file, line)))
    {
        return;
    }

    FILE* file_out = fopen("output.txt", "a");
    if (!file_out)
    {
        fprintf(err_list, "ERROR: can't open output.txt\n");
        return;
    }

    for (size_t i = 0; i < stk->size; i++)
    {
        if (i > 0) fprintf(file_out, " ");
        fprintf(file_out, "%lg", stk->data[i]);
    }
    fprintf(file_out, "\n");

    fclose(file_out);
}


void StackClear_(Stack_s* stk
                 ON_DBG(, const char* stk_name, const char* file, int line))
{
    int err = StackOK_(stk ON_DBG(, stk_name, file, line));
    if (err == NULL_PTR_ERR)
    {
        return;
    }

    for (size_t i = 0; !err && i < stk->size; i++)
    {
        stk->data[i] = POISON;
    }

    free(stk->data);

    stk->data = NULL;
    stk->size = 0;
    stk->capacity = 0;
}
