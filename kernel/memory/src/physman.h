#ifndef PHYSMAN_H
#define PHYSMAN_H

#include <types.h>
#include <alien/kernel.h>
#include <alien/memory/paging.h>
#include <errno.h>

typedef struct _physman physman_t;

static const errno_t physman_out_of_memory = -10;


ppage_t         physman_allocate_page   (physman_t* m);

physman_t*      physman_init            (const kernel_info_t* info);
unsigned int    physman_used_page_count (const physman_t *m);
bool            physman_is_page_used    (const physman_t* m, ppage_t page);
errno_t		physman_errno		(const physman_t *m);

#endif
