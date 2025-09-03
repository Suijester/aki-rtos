#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "semaphore.h"
struct semaphore;
// taskMethod is a pointer to any function/task
typedef void (*taskPtr)(void);

typedef enum {
	READY,
	BLOCKED
} taskState;

typedef struct taskController {
	uint32_t* taskStackPtr; // stack frame
	uint8_t priority; // scale from 0 to 10, 10 is highest priority
	taskState currentState;
	semaphore* waitingFor;

	struct taskController* prev;
	struct taskController* next;
} taskController;

// scheduler functions
taskController* addTask(taskPtr taskProvided, uint8_t taskPriority); // add any task to be managed
void startScheduler(void); // begin scheduling

void addReadyTask(taskController* task);
void removeReadyTask(taskController* task);

void schedulerYield();
void schedulerBlockCurrent(semaphore* sem);
int schedulerUnblockTask(semaphore* sem);

#endif
