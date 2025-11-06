// Simple deadlock using semaphores

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t s1, s2;

void *thread1(void *arg)
{
    printf("T1: waiting for s1\n");
    sem_wait(&s1);
    printf("T1: acquired s1\n");
    sleep(1);
    printf("T1: waiting for s2\n");
    sem_wait(&s2);
    printf("T1: acquired s2 (this won't print because of deadlock)\n");
    sem_post(&s2);
    sem_post(&s1);
    return NULL;
}

void *thread2(void *arg)
{
    printf("T2: waiting for s2\n");
    sem_wait(&s2);
    printf("T2: acquired s2\n");
    sleep(1);
    printf("T2: waiting for s1\n");
    sem_wait(&s1);
    printf("T2: acquired s1 (this won't print because of deadlock)\n");
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