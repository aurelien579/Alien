/*******************************************************************************
 * SOURCE NAME  : math.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide math functions and macros
 ******************************************************************************/

#ifndef MATH_H
#define MATH_H

#define updiv(a, b)     \
(((a) + (b) - 1) / (b)) \

#define max(a, b)               \
({ __typeof__ (a) _a = (a);     \
    __typeof__ (b) _b = (b);    \
    _a > _b ? _a : _b; })

#define min(a, b)               \
({ __typeof__ (a) _a = (a);     \
    __typeof__ (b) _b = (b);    \
    _a < _b ? _a : _b; })

#endif