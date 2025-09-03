/*
 * semaphore.c
 *
 *  Created on: Sep 1, 2025
 *      Author: akisub
 */

#include "semaphore.h"
#include "scheduler.h"

void semaphoreConstructor(semaphore* sem, uint8_t resources) {
	sem->availableResources = resources;
}

void semaphoreBlock(semaphore* sem) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	if (sem->availableResources > 0) {
		sem->availableResources--;
	} else {
		schedulerBlockCurrent(sem);
		schedulerYield();
	}

	__set_PRIMASK(savedPrimask);
}

void semaphorePost(semaphore* sem) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	if (schedulerUnblockTask(sem)) {
		schedulerYield();
	} else {
		sem->availableResources++;
	}

	__set_PRIMASK(savedPrimask);
}

int semaphoreTry(semaphore* sem) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	int success = 0;

	if (sem->availableResources > 0) {
		sem->availableResources--;
		success = 1;
	}

	__set_PRIMASK(savedPrimask);
	return success;
}

void semaphorePostISR(semaphore* sem) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	schedulerUnblockTask(sem);
	sem->availableResources++;

	__set_PRIMASK(savedPrimask);
}
