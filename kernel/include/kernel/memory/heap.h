#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

void heap_install();
void *kmalloc(uint32_t size);
void kfree(void *ptr);

#endif