/*******************************************************************************
 * SOURCE NAME  : string.h
 * VERSION      : 0.1
 * CREATED DATE : 07/02/2018
 * LAST UPDATE  : 07/02/2018
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides functions for manipulating strings.
 ******************************************************************************/

#ifndef LIB_STRING_H
#define LIB_STRING_H

struct strarray
{
    unsigned int size;
    char *array[];
};

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *str, unsigned int n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char* src, unsigned int n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);

unsigned int strlen(const char *s);

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);
void *memsetw(void *s, int c, unsigned int n);

char* itoa(int value, char* result, int base);

struct strarray *strsplit(const char *str, char c);
struct strarray *strarray_new(unsigned int size);
void strarray_free(struct strarray *a);

#endif
