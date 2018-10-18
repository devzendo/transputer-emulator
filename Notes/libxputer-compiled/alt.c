/*
 *   alt.c  (libxputer.a)
 *
 *   October 22, 1996, Roman Pozlevich <roma@botik.ru>
 *   
 *   Transputer alternative input.
 */

#include <stddef.h>
#include <xputer.h>


/*
 *  Pure ALT implementation
 *

channel *alt (channel *first, ...) {
    channel *res, **ptr;

    __asm__ __volatile__ (
	"  alt                           \n\t"
	"  ldlp %w2; stl %w1             \n\t" 
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; ldc 1; enbc  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
        "  altwt                         \n\t"
	"  ldlp %w2; stl %w1             \n\t" 
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; dup; ldc 1; rev; disc \n\t"
	"  eqc 0; cj 1f                  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
	"  ldl 0                         \n\t"

	: "=a" (res)
	: "m" (ptr), "m" (first)
	: "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]"
    );
    return res;
}

*/


int __alt (int skip, channel_t **clist) {
    channel_t **res, **tmp;

    __asm__ __volatile__ (
	"  alt                           \n\t"
	"  ldl %w2; stl %w1              \n\t" 
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; ldc 1; enbc  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
        "  ldl %w3; enbs                 \n\t"
        "  altwt                         \n\t"
	"  ldl %w2; stl %w1              \n\t" 
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; ldc 1; ldl %w1; disc \n\t"
	"  eqc 0; cj 1f                  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
	"  ldc %w3; ldc 0; diss          \n\t"
	"  ldl 0                         \n\t"

	: "=a" (res)
	: "m" (tmp), "m" (clist), "m" (skip)
	: "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]"
    );
    return (res)? res - clist : -1;
}


int __talt (int ticks, channel_t *clist[]) {
    channel_t **res, **tmp;

    __asm__ __volatile__ (
	"  talt                          \n\t"
	"  ldl %w3; ldc 1; enbt          \n\t"
	"  ldl %w2; stl %w1              \n\t"
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; ldc 1; enbc  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
        "  taltwt                        \n\t"
	"  ldl %w2; stl %w1              \n\t" 
        "0:                              \n\t"
        "  ldl %w1; ldnl 0; cj 1f        \n\t"
        "  ldl %w1; ldnl 0; ldc 1; ldl %w1; disc \n\t"
	"  eqc 0; cj 2f                  \n\t"
        "  ldl %w1; ldnlp 1; stl %w1     \n\t"
        "  ldc 0; cj 0b                  \n\t"
        "1:                              \n\t"
        "  ldl %w3; ldc 1; ldc 0; dist   \n\t"
        "2:                              \n\t"
	"  ldl 0                         \n\t"

	: "=a" (res)
	: "m" (tmp), "m" (clist), "m" (ticks)
	: "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]"
    );
    return (res)? res - clist : -1;
}


/*
 *  Timer overflow periods:
 *     4295 seconds (71 minutes) for high priority;
 *     274877 seconds (76 hours) for low priority.
 *
 *  Two timer values can be compared only if they are both contained
 *  within a half cycle.  So, never make `talt' sleep longer than
 *
 *     32        31
 *    2  / 2 == 2  == 2147483648 ticks
 *
 *  TO DO:
 *    - update timeout value to reflect the amount of time remaining
 */

#define	MAXTICKS	2100000000UL

int alt (struct timeval *timeout, channel_t *clist[]) {
    int res;
    long long ticks;
    unsigned timer = process_timer ();

    if (!timeout)
	return __alt (0, clist);
    if (!timeout->tv_sec && !timeout->tv_usec)
	return __alt (1, clist);

    ticks = process_priority () ?
                timeout->tv_sec*1000000/64 + timeout->tv_usec/64 :
                timeout->tv_sec*1000000    + timeout->tv_usec;

    while (ticks >= MAXTICKS) {
	ticks -= MAXTICKS;
	timer += MAXTICKS;
	res = __talt (timer, clist);
	if (res >= 0)
	    return res;
    }

    if (!ticks)
	return -1;

    return __talt (timer + ticks, clist);
}

