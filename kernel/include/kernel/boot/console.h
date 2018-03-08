/*******************************************************************************
 * SOURCE NAME  : console.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements a VERY basic console used during initializion stage.
 ******************************************************************************/

#ifndef BOOT_CONSOLE_H
#define BOOT_CONSOLE_H

/**
 * A very basic vga console used during boot stage
 */


/**
 * @brief Clear the vga buffer
 */
void boot_clear();


/**
 * @brief Write a character
 * @param c
 */
void boot_putchar(char c);


/**
 * @brief Write a null-terminated string
 * @param s
 */
void boot_print(const char *s);

#endif

