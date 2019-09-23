/*******************************************************************************
 * SOURCE NAME  : paging.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides functions for managing pages
 ******************************************************************************/

#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096

void paging_install(uint32_t total_mem, uint32_t kernel_len);

uint32_t alloc_kpage();
uint32_t alloc_page(uint32_t offset, uint32_t is_user);
uint32_t alloc_continuous_pages(uint32_t n);
void free_page(uint32_t page);

void switch_page_dir(uint32_t dir);

uint32_t create_user_page_dir();

#endif
