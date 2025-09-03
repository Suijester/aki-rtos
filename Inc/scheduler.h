#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
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
	struct semaphore* waitingFor;

	struct taskController* prev;
	struct taskController* next;
} taskController;

// scheduler functions
void addTask(taskPtr taskProvided, uint8_t taskPriority); // add any task to be managed
void startScheduler(void); // begin scheduling


void schedulerYield();
void schedulerBlockCurrent(struct semaphore* sem);
int schedulerUnblockTask(struct semaphore* sem);

#endif
