/*******************************************************************************
 * SOURCE NAME  : string.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements functions for manipulating strings.
 ******************************************************************************/

char* strcat(char *dest, const char *src)
{
    char* ret = dest;
    while (*dest != '\0')
        dest++;
     while ((*dest++ = *src++))
        ;
    return ret;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1==*s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char *strcpy(char *dest, const char* src)
{
    char *ret = dest;
    while ((*dest++ = *src++)) ;
    return ret;
}

unsigned int strlen(const char *s)
{
    unsigned int i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}

char *strncat(char *dest, const char *src, unsigned int n)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while (n--)
        if (!(*dest++ = *src++))
            return ret;
    *dest = 0;
    return ret;
}

int strncmp(const char* s1, const char* s2, unsigned int n)
{
    while (n--)
        if (*s1++ != *s2++)
            return *(unsigned char*) (s1 - 1) - *(unsigned char*) (s2 - 1);
    return 0;
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

void *memset(void *s, int c, unsigned int n)
{
    unsigned char* p = s;
    while (n--)
        *p++ = (unsigned char) c;
    return s;
}

void *memsetw(void *s, int c, unsigned int n)
{
    unsigned short* p = s;
    while (n--)
        *p++ = (unsigned short) c;
    return s;
}

char* itoa(int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) {
        *result = '\0';
        return result;
    }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return result;
}
