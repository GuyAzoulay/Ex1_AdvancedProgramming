#include "queue.h"
#include <stdlib.h>
#include <string.h>

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    pthread_mutex_init(&(q->lock), NULL);
    return q;
}

bool isEmpty(Queue* q) {
    return (q->size == 0);
}

bool isFull(Queue* q) {
    return (q->size == MAX_QUEUE_SIZE);
}

void enqueue(Queue* q, char* message, int index) {
    pthread_mutex_lock(&(q->lock));
    if (isFull(q)) {
        pthread_mutex_unlock(&(q->lock));
        return;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    Message m;
    strcpy(m.message, message);
    m.index = index;
    q->messages[q->rear] = m;
    q->size++;
    pthread_mutex_unlock(&(q->lock));
}

Message dequeue(Queue* q) {
    pthread_mutex_lock(&(q->lock));
    if (isEmpty(q)) {
        pthread_mutex_unlock(&(q->lock));
        Message emptyMessage;
        emptyMessage.message[0] = '\0';
        emptyMessage.index = -1;
        return emptyMessage;
    }
    Message m = q->messages[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    pthread_mutex_unlock(&(q->lock));
    return m;
}
