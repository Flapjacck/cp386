// Simple deadlock using semaphores

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t s1, s2;

static void acquire_two_with_preemption(sem_t *first, sem_t *second, const char *tname)
{
    for (;;)
    {
        printf("%s: waiting for first\n", tname);
        sem_wait(first);
        printf("%s: acquired first\n", tname);

        /* try to get second without blocking; if unsuccessful, release first and retry */
        printf("%s: trying to acquire second\n", tname);
        if (sem_trywait(second) == 0)
        {
            printf("%s: acquired second\n", tname);
            return; /* both acquired */
        }

        /* couldn't get second -> release first and back off */
        printf("%s: couldn't acquire second, releasing first and backing off\n", tname);
        sem_post(first);
        usleep(100000); /* 100ms backoff; adjust as needed */
    }
}

void *thread1(void *arg)
{
    acquire_two_with_preemption(&s1, &s2, "T1");
    /* critical section */
    printf("T1: in critical section with both semaphores\n");
    sem_post(&s2);
    sem_post(&s1);
    return NULL;
}

void *thread2(void *arg)
{
    acquire_two_with_preemption(&s2, &s1, "T2");
    /* critical section */
    printf("T2: in critical section with both semaphores\n");
    sem_post(&s1);
    sem_post(&s2);
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;

    sem_init(&s1, 0, 1);
    sem_init(&s2, 0, 1);

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&s1);
    sem_destroy(&s2);
    printf("Done\n");
    return 0;
}