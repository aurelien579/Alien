/*******************************************************************************
 * SOURCE NAME  : debug.h
 * VERSION      : 0.1
 * CREATED DATE : 07/02/2018
 * LAST UPDATE  : 07/02/2018
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides debugging functions.
 ******************************************************************************/

#ifndef DEBUG_DEBUG_H
#define DEBUG_DEBUG_H

#include <kernel/cpu.h>
#include <stdio.h>

#define KERNEL_PANIC(msg)                                                       \
do {                                                                            \
    printf("[PANIC] at %s:%d : %s\n", __FILE__, __LINE__, msg);                 \
    while (1);                                                                  \
} while (0);                                                                    \

void dump_regs(const struct regs *regs);

#endif
