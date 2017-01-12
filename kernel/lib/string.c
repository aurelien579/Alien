void strcat(char *dest, const char *src)
{
    char* ret = dest;
    while (*dest != '\0')
        dest++;
     while (*dest++ = *src++)
        ;
    return ret;
}

int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char *strcpy(char *dest, const char* src)
{
    char *ret = dest;
    while (*dest++ = *src++)
        ;
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
    while(n--)
        if(*s1++ != *s2++)
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
    while(n--)
        *p++ = (unsigned char) c;
    return s;
}
