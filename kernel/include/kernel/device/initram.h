/*******************************************************************************
 * SOURCE NAME  : initram.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Exports public functions of the initram driver. This driver
 * is trivial. It just read/write to RAM.
 ******************************************************************************/

#ifndef DEVICE_INITRAM_H
#define DEVICE_INITRAM_H

void initram_install(uint32_t base, uint32_t length);

#endif
