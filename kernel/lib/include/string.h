#ifndef LIB_STRING_H
#define LIB_STRING_H

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *str, unsigned int n);

char *strcpy(char *dest, const char *src);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);

unsigned int strlen(const char *s);

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);
void *memsetw(void *s, int c, unsigned int n);

char* itoa(int value, char* result, int base);

#endif
