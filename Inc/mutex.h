/*
 * mutex.h
 *
 *  Created on: Sep 2, 2025
 *      Author: akisub
 */

#ifndef INC_MUTEX_H_
#define INC_MUTEX_H_

struct semaphore;
struct taskController;

typedef struct {
	struct semaphore binarySem;
	struct taskController* owner;
	uint8_t initialOwnerPriority;
} mutex;

void mutexConstructor(mutex* mutex);
void mutexLock(mutex* mutex, taskController* tcb);
void mutexUnlock(mutex* mutex, taskController* tcb);


#endif /* INC_MUTEX_H_ */
