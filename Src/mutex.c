/*
 * mutex.c
 *
 *  Created on: Sep 2, 2025
 *      Author: akisub
 */

#include "mutex.h"
#include "semaphore.h"
#include "scheduler.h"

void mutexConstructor(mutex* mutex) {
	semaphoreConstructor(&mutex->binarySem);
	mutex->initialOwnerPriority = -1;
	mutex->owner = NULL;
}

void mutexLock(mutex* mutex, taskController* tcb) {
	if (semaphoreTry(&mutex->binarySem) == 0) {
		if (tcb->priority > mutex->owner->priority) {
			mutex->owner->priority = tcb->priority;
		}
		semaphoreBlock(&mutex->binarySem);
		mutex->owner = tcb; // when this task wakes up, it immediately becomes the owner, since the mutex got released
		mutex->initialOwnerPriority = tcb->priority;
	} else {
		mutex->owner = tcb;
		mutex->initialOwnerPriority = tcb->priority;
	}
}

void mutexUnlock(mutex* mutex, taskController* tcb) {
	if (mutex->owner == NULL || mutex->owner != tcb) {
		return; // either mutex is already unlocked or another task is trying to unlock the mutex.
	}
	mutex->owner->priority = mutex->initialOwnerPriority;
	mutex->initialOwnerPriority = -1;
	mutex->owner = NULL;
	semaphorePost(&mutex->binarySem);
}
