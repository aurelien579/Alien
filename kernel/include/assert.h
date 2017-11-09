#ifndef ASSERT_H
#define ASSERT_H

#include <alien/io.h>

#define assert(expr)\
	do {\
		if (!(expr)) {\
			kprintf("assertion failed at "__FILE__":%d\n", __LINE__);\
			while(1);\
		}\
	} while(0);

#endif
