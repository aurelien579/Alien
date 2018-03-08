/*******************************************************************************
 * SOURCE NAME  : kernel.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide some informations about the kernel. Static infos are
 *  accessible through macros and dynamic ones through KERNEL structure. The
 *  KERNEL structure is initialized right after boot.
 ******************************************************************************/

#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include <stdint.h>

extern uint32_t __KERNEL_START__;
extern uint32_t __KERNEL_END__;
extern uint32_t __KERNEL_SIZE__;
extern uint32_t __KERNEL_VBASE__;

#define KERNEL_START    ((uint32_t) &__KERNEL_START__)
#define KERNEL_END      ((uint32_t) &__KERNEL_END__)
#define KERNEL_SIZE     ((uint32_t) &__KERNEL_SIZE__)
#define KERNEL_VBASE    ((uint32_t) &__KERNEL_VBASE__)

struct kernel
{
    uint32_t memlen;
};

extern struct kernel KERNEL;

#endif
