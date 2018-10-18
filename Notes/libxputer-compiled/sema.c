/*
 *   sema.c  (libxputer.a)
 *
 *   October 22, 1996, Roman Pozlevich <roma@botik.ru>
 *
 *   Software version of T9000/T450 semaphores.
 *   (it will be some day... really... ;-)
 */

#include <xputer.h>


void __semaphore_wait (sema_t *sem) {
    volatile unsigned *count = &sem->count;

    while (*count == 0)
	process_deschedule ();
    (*count)--;
}


void __semaphore_signal (sema_t *sem) {
    volatile unsigned *count = &sem->count;

    (*count)++;
}

