/*******************************************************************************
 * SOURCE NAME  : strarr.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides basic string array implementation
 ******************************************************************************/

#include <string.h>
#include <kernel/memory/heap.h>

struct strarray *
strsplit(const char *str, char c)
{
    unsigned int count = 1;
    const char *ptr = str;

    /* Skip delimiters at the beginning of the string */
    while (*ptr == c) {
        ptr++;
    }

    /* Count number of elems (ie. the number of delimiters found). 
       If the string ends with a delimiter, count is not incremented so that
       count represents the number of output strings.
    */
    while (*ptr) {
        if (*ptr != c) {
            ptr++;
            continue;
        }

        count++;

        while (*ptr == c) {
            ptr++;

            if (!*ptr) {
                count--;
                break;
            }
        }
    }

    struct strarray *a = strarray_new(count);
    unsigned int current = 0;

    /* 'ptr' iterate over the string 'str' */
    ptr = str;
    while (*ptr) {
        /* Skip all ocurences of the delimiter */
        while (*ptr == c) {
            ptr++;
        }

        unsigned int current_size = 0;
        const char *current_ptr = ptr;
        while (*current_ptr != c && *current_ptr) {
            current_size++;
            current_ptr++;
        }


        char *s = (char *) kmalloc(sizeof(char) * (current_size + 1));
        strncpy(s, ptr, current_size);
        s[current_size] = 0;

        a->array[current] = s;

        ptr += current_size;
        current++;
    }

    return a;
}

struct strarray *
strarray_new(unsigned int size)
{
    unsigned int total_size = sizeof(struct strarray) + sizeof(char *) * size;
    struct strarray *a = (struct strarray *) kmalloc(total_size);

    memset(a, 0, total_size);
    a->size = size;
    return a;
}

void
strarray_free(struct strarray *a)
{
    if (!a) return;

    for (unsigned int i = 0; i < a->size; i++) {
        if (a->array[i]) {
            kfree((void *) a->array[i]);
        }
    }

    kfree((void *) a);
}
