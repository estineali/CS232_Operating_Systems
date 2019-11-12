/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2019.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* prod_condvar.c

   A simple POSIX threads producer-consumer example using a condition variable.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int avail = 0;

static void *
threadFunc(void *arg)
{
    int cnt = atoi((char *) arg);
    int s, j;

    for (j = 0; j < cnt; j++) {
        sleep(1);

        /* Code to produce a unit omitted */

        s = pthread_mutex_lock(&mtx);

        avail++;        /* Let consumer know another unit is available */

        s = pthread_mutex_unlock(&mtx);

        s = pthread_cond_signal(&cond);         /* Wake sleeping consumer */
    }

    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t tid;
    int s, j;
    int totRequired;            /* Total number of units that all threads
                                   will produce */
    int numConsumed;            /* Total units so far consumed */
    int done;
    time_t t;

    t = time(NULL);

    /* Create all threads */

    totRequired = 0;
    for (j = 1; j < argc; j++) {
        totRequired += atoi(argv[j]);

        s = pthread_create(&tid, NULL, threadFunc, argv[j]);
    }

    /* Loop to consume available units */

    numConsumed = 0;
    done = 0;

    for (;;) {
        s = pthread_mutex_lock(&mtx);

        while (avail == 0) {            /* Wait for something to consume */
            s = pthread_cond_wait(&cond, &mtx);
        }

        /* At this point, 'mtx' is locked... */

        while (avail > 0) {             /* Consume all available units */

            /* Do something with produced unit */

            numConsumed ++;
            avail--;
            printf("T=%ld: numConsumed=%d\n", (long) (time(NULL) - t),
                    numConsumed);

            done = numConsumed >= totRequired;
        }

        s = pthread_mutex_unlock(&mtx);

        if (done)
            break;

        /* Perhaps do other work here that does not require mutex lock */

    }

    exit(EXIT_SUCCESS);
}

