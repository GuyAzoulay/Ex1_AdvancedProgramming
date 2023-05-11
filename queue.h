#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 1024

typedef struct {
    char message[MAX_QUEUE_SIZE];
    int index;
} Message;

typedef struct {
    Message messages[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
    pthread_mutex_t lock;
} Queue;

Queue* createQueue();
bool isEmpty(Queue* q);
bool isFull(Queue* q);
void enqueue(Queue* q, char* message, int index);
Message dequeue(Queue* q);

#endif /* QUEUE_H */
