#ifndef STRING_H
#define STRING_H

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);;
void *memsetw(void *s, int c, unsigned int n);

char* strcat(char *dest, const char *src);
int strcmp(const char* s1, const char* s2);
char *strcpy(char *dest, const char* src);
unsigned int strlen(const char *s);
char *strncat(char *dest, const char *src, unsigned int n);
int strncmp(const char* s1, const char* s2, unsigned int n);

char* itoa(int value, char* result, int base);

#endif
