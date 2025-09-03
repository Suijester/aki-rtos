/*
 * messageQueue.h
 *
 *  Created on: Sep 1, 2025
 *      Author: akisub
 */

#ifndef INC_MESSAGEQUEUE_H_
#define INC_MESSAGEQUEUE_H_

struct semaphore;

typedef struct {
	void* dataAddress;
	uint8_t typeSize;
	uint8_t queueCapacity;
	uint8_t head;
	uint8_t tail;

	struct semaphore itemsAvailable;
	struct semaphore spacesAvailable;
	struct semaphore mutex; // if parallel execution occurs

} messageQueue;

void queueConstructor(messageQueue* queue, void* dataLocation, uint8_t typeSize, uint8_t queueCapacity);

void queueReset(messageQueue* queue);
void queuePeek(messageQueue* queue, void* destination); // queueReceive without removing item

int queueEmpty(messageQueue* queue);
int queueFull(messageQueue* queue);

void queueSend(messageQueue* queue, const void* item);
int queueSendISR(messageQueue* queue, const void* item);

void queueReceive(messageQueue* queue, void* destination);
int queueReceiveISR(messageQueue* queue, void* destination);

#endif /* INC_MESSAGEQUEUE_H_ */
