/*
 * CircularQueue.h
 *
 *  Created on: Sep 29, 2015
 *      Author: DebashisGanguly
 */

#ifndef GLOBAL_ADT_CIRCULARQUEUE_H_
#define GLOBAL_ADT_CIRCULARQUEUE_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct _circularQueue {
	int head;
	int tail;
	int count;
	int size;
	void **items;
} CircularQueue;

static inline CircularQueue *createCircularQueue (int size) {
	CircularQueue *circularQueue = malloc (sizeof(CircularQueue));

	circularQueue -> head = 0;
	circularQueue -> tail = 0;
	circularQueue -> count = 0;
	circularQueue -> size = size;
	circularQueue -> items = (void **) malloc (size * sizeof(void *));

	return circularQueue;
}

static inline int getCountCircularQueue (CircularQueue *circularQueue) {
	if (circularQueue == NULL)
		return 0;
	else
		return circularQueue -> count;
}

static inline int isFullCircularQueue (CircularQueue *circularQueue) {
	if (circularQueue == NULL)
		return -1;
	else
		return circularQueue -> size == circularQueue -> count;
}

static inline void enqueueCircular (CircularQueue *circularQueue, void *value) {
	if (!isFullCircularQueue (circularQueue)) {
		circularQueue -> items [circularQueue -> tail] = value;
		circularQueue -> tail = (circularQueue -> tail + 1) % circularQueue -> size;
		circularQueue -> count++;
	}
}

static inline void *dequeueCircular (CircularQueue *circularQueue) {
	void *value = NULL;

	if (getCountCircularQueue (circularQueue) != 0) {
		value = circularQueue -> items [circularQueue -> head];
		circularQueue -> head = (circularQueue -> head + 1) % circularQueue -> size;
		circularQueue -> count--;
	}

	return value;
}

static inline void *getHeadCircularQueue (CircularQueue *circularQueue) {
	void *value = NULL;

		if (getCountCircularQueue (circularQueue) != 0) {
			value = circularQueue -> items [circularQueue -> head];
		}

		return value;
}

static inline int isValidCircularIndex (CircularQueue *circularQueue, int index) {
    if (circularQueue -> head < circularQueue -> tail) {
        return (index >= circularQueue -> head && index <= circularQueue -> tail);
    } else {
        return (index >= circularQueue->head && index < circularQueue->size) || (index <= circularQueue->tail);
    }
}

#endif /* GLOBAL_ADT_CIRCULARQUEUE_H_ */
