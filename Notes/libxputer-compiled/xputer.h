/*
 *   xputer.h
 *
 *   This is the header file for libxputer.a v0.05
 *   October 22, 1996, Roman Pozlevich <roma@botik.ru>
 * 
 *   Transputer specific features.
 *   Most are impemented as inline C functions.
 */


#ifndef __XPUTER_H
#define __XPUTER_H	1

#include <stddef.h>

typedef	unsigned	channel_t;
typedef	unsigned	pdesc_t;

typedef struct {
    unsigned	count;
    pdesc_t	front;	/* front of waiting queue */
    pdesc_t	back;	/* back of waiting queue */
} sema_t;


/* Some day `timeval' definition will be moved to proper location */

struct timeval {
    long tv_sec;        /* seconds */
    long tv_usec;  /* microseconds */
};


#define	__MINT		0x80000000u
#define	NOTPROCESS	((pdesc_t) __MINT)
#define	LINK0OUTPUT	((channel_t *) __MINT)
#define	LINK1OUTPUT	((channel_t *) __MINT + 1)
#define	LINK2OUTPUT	((channel_t *) __MINT + 2)
#define	LINK3OUTPUT	((channel_t *) __MINT + 3)
#define	LINK0INPUT	((channel_t *) __MINT + 4)
#define	LINK1INPUT	((channel_t *) __MINT + 5)
#define	LINK2INPUT	((channel_t *) __MINT + 6)
#define	LINK3INPUT	((channel_t *) __MINT + 7)
#define	EVENT		((channel_t *) __MINT + 8)


static __inline__ int process_priority (void) {
    int tmp;

    __asm__ __volatile__ ( "ldpri" : "=a" (tmp) : );
    return tmp;
}


static __inline__ unsigned process_timer (void) {
    int tmp;

    __asm__ __volatile__ ( "ldtimer" : "=a" (tmp) : );
    return tmp;
}


static __inline__ void process_wait (unsigned ticks) {
    __asm__ __volatile__ ( 
	"tin" 
        : 
        : "aP" (ticks)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg"
    );
}


static __inline__ void process_deschedule (void) {
    process_wait (process_timer ());
}


static __inline__ void process_stop (void) {
    __asm__ __volatile__ (
        "stopp"
        : 
        : 
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "memory"
    );
}


static __inline__ void process_start (void *workspace, void (*process_ptr) ()) {
    __asm__ __volatile__ ( 
        "  ldc 9f-8f  \n\t"
	"  ldpi       \n\t"
	"8:           \n\t"
        "  diff       \n\t"
        "  rev        \n\t"
        "  startp;    \n\t"
        "9: "
        :
        : "bP" (workspace), "aP" (process_ptr)
        : "Areg", "Breg", "Creg"
    );
}


static __inline__ void process_run (pdesc_t desc) {
    __asm__ __volatile__ ( "runp" : : "aP" (desc) : "Areg" );
}


static __inline__ pdesc_t process_self (void) {
    pdesc_t p;

    __asm__ __volatile__ ( "ldlp 0" : "=a" (p) : );
    return p | process_priority ();
}


static __inline__ void __chan_out (channel_t *chan, void *data, size_t size) {
    __asm__ __volatile__ (
        "out"
        : 
        : "cP" (data), "bP" (chan), "aP" (size)
        : "FAreg", "FBreg", "FCreg"
    );
}


static __inline__ void __chan_outword (channel_t *chan, int data) {
    __asm__ __volatile__ ( 
        "outword" 
        : 
        : "bP" (chan), "aP" (data)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]"
    );
}


static __inline__ void __chan_outbyte (channel_t *chan, char data) {
    __asm__ __volatile__ ( 
        "outbyte" 
        : 
        : "bP" (chan), "aP" (data)
        : "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]"
    );
}


static __inline__ void __chan_in (channel_t *chan, void *data, size_t size) {
    __asm__ __volatile__ ( 
        "in" 
        : 
        : "cP" (data), "bP" (chan), "aP" (size)
        : "FAreg", "FBreg", "FCreg", "memory"
    );
}


static __inline__ pdesc_t __chan_reset (channel_t *chan) {
    pdesc_t res;

    __asm__ __volatile__ ( "resetch" : "=a" (res) : "aP" (chan) );
    return res;
}


static __inline__ void *__memcpy (void *dest, void *src, size_t size) {
    __asm__ __volatile__ ( 
        "move" 
        : 
        : "cP" (src), "bP" (dest), "aP" (size)
        : "FAreg", "FBreg", "FCreg", "memory"
    );
    return dest;
}


static __inline__ void semaphore_init (sema_t *sem, int n) {
    sem->front = NOTPROCESS;
    sem->count = n;
}


static __inline__ void semaphore_wait (sema_t *sem) {
#ifndef	HASWAIT
    extern void __semaphore_wait (sema_t *);
    __semaphore_wait (sem);
#else
    __asm__ __volatile__ (
        "wait"
        : 
        : "aP" (sem)
        : "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "memory"
    );
#endif
}

static __inline__ void semaphore_signal (sema_t *sem) {
#ifndef	HASWAIT
    extern void __semaphore_signal (sema_t *);
    __semaphore_signal (sem);
#else
    __asm__ __volatile__ (
        "signal"
        : 
        : "aP" (sem)
        : "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "memory"
    );
#endif
}


static __inline__ int AFTER (unsigned x, unsigned y) {
    return ((int) (x - y)) > 0;
}


extern int 	__alt (int skip, channel_t *chans[]);
extern int 	__talt (int ticks, channel_t *chans[]);
extern int 	alt (struct timeval *tm, channel_t *chans[]);

extern void	process_tohigh (void);
extern void	process_tolow (void);

extern int	lddevid (void);


#endif	/* __XPUTER_H */

