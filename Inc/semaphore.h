/*
 * semaphore.h
 *
 *  Created on: Sep 1, 2025
 *      Author: akisub
 */

#ifndef INC_SEMAPHORE_H_
#define INC_SEMAPHORE_H_

#include <stdint.h>

typedef struct {
	volatile uint8_t availableResources; // can access if availableResources > 0
} semaphore;

void semaphoreConstructor(semaphore* sem, uint8_t resources);
void semaphorePost(semaphore* sem);
void semaphoreBlock(semaphore* sem);

int semaphoreTry(semaphore* sem); // nonblocking attempt
void semaphorePostISR(semaphore* sem); // nonblocking/no context switch post



#endif /* INC_SEMAPHORE_H_ */
