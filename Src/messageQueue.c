/*
 * messageQueue.c
 *
 *  Created on: Sep 1, 2025
 *      Author: akisub
 */

#include "messageQueue.h"
#include <string.h>
#include "stm32f4xx.h"
#include "core_cm4.h"

void queueConstructor(messageQueue* queue, void* dataLocation, uint8_t typeSize, uint8_t queueCapacity) {
	queue->dataAddress = dataLocation;
	queue->typeSize = typeSize;
	queue->queueCapacity = queueCapacity;

	queue->head = 0;
	queue->tail = 0;

	semaphoreConstructor(&queue->spacesAvailable, queueCapacity);
	semaphoreConstructor(&queue->itemsAvailable, 0);
	semaphoreConstructor(&queue->mutex, 1);
}

void queueSend(messageQueue* queue, const void* item) {
	semaphoreBlock(&queue->spacesAvailable);
	semaphoreBlock(&queue->mutex);

	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	void* destination = (uint8_t*)queue->dataAddress + (queue->tail * queue->typeSize);
	memcpy(destination, item, queue->typeSize);
	queue->tail = (queue->tail + 1) % queue->queueCapacity;

	__set_PRIMASK(savedPrimask);

	semaphorePost(&queue->mutex);
	semaphorePost(&queue->itemsAvailable);
}

int queueSendISR(messageQueue* queue, const void* item) {
	if (!semaphoreTry(&queue->spacesAvailable)) {
		return 0;
	} else {
		uint32_t savedPrimask = __get_PRIMASK();
		__disable_irq();

		void* destination = (uint8_t*)queue->dataAddress + (queue->tail * queue->typeSize);
		memcpy(destination, item, queue->typeSize);
		queue->tail = (queue->tail + 1) % queue->queueCapacity;

		semaphorePostISR(&queue->itemsAvailable);
		__set_PRIMASK(savedPrimask);

		return 1;
	}
}

void queueReceive(messageQueue* queue, void* destination) {
	semaphoreBlock(&queue->itemsAvailable);
	semaphoreBlock(&queue->mutex);

	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	const void* item = (uint8_t*)queue->dataAddress + (queue->head * queue->typeSize);
	memcpy(destination, item, queue->typeSize);
	queue->head = (queue->head + 1) % queue->queueCapacity;

	__set_PRIMASK(savedPrimask);

	semaphorePost(&queue->mutex);
	semaphorePost(&queue->spacesAvailable);
}

int queueReceiveISR(messageQueue* queue, void* destination) {
	if (!semaphoreTry(&queue->itemsAvailable)) {
		return 0;
	} else {
		uint32_t savedPrimask = __get_PRIMASK();
		__disable_irq();

		const void* item = (uint8_t*)queue->dataAddress + (queue->head * queue->typeSize);
		memcpy(destination, item, queue->typeSize);
		queue->head = (queue->head + 1) % queue->queueCapacity;

		semaphorePostISR(&queue->spacesAvailable);
		__set_PRIMASK(savedPrimask);

		return 1;
	}
}

void queueReset(messageQueue* queue) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	queue->head = 0;
	queue->tail = 0;

	semaphoreConstructor(&queue->spacesAvailable, queue->queueCapacity);
	semaphoreConstructor(&queue->itemsAvailable, 0);
	semaphoreConstructor(&queue->mutex, 1);

	__set_PRIMASK(savedPrimask);
}

void queuePeek(messageQueue* queue, void* destination) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	if (queue->head != queue->tail) {
		const void* item = (uint8_t*)queue->dataAddress + (queue->head * queue->typeSize);
		memcpy(destination, item, queue->typeSize);
	}

	__set_PRIMASK(savedPrimask);
}

int queueEmpty(messageQueue* queue) {
	return (queue->head == queue->tail);
}

int queueFull(messageQueue* queue) {
	return ((queue->tail + 1) % queue->queueCapacity == queue->head);
}


