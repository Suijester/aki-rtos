#include "scheduler.h"
#include "stm32f4xx.h"
#include "core_cm4.h"
#include <stddef.h>

#define MAX_TASKS 5
#define STACK_SIZE 128
#define CLK_TICK_SPEED 1000
#define PRIORITY_LEVELS 11 // 11 levels, from 0 to 10 (0 is reserved for idle task)

static taskController* taskList[MAX_TASKS]; // access method for task controllers & stack frames
static taskController tcbStorage[MAX_TASKS]; // storage array to store task controllers
static uint32_t taskStacks[MAX_TASKS][STACK_SIZE]; // 2D storage array to store stack frames

static taskController* readyLists[PRIORITY_LEVELS];
static uint32_t readyBitmap = 0;


static uint8_t tasksManaged = 0; // number of tasks total

static taskController* currentTaskController;
static taskController* idleTaskController;

void addReadyTask(taskController* task) {
	task->next = NULL;
	task->prev = NULL;

	if (readyLists[task->priority] == NULL) {
		readyLists[task->priority] = task;
	} else {
		taskController* temp = readyLists[task->priority];
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = task;
		task->prev = temp;
	}

	readyBitmap |= (1 << task->priority);
}

void removeReadyTask(taskController* task) {
	if (task->prev) {
		task->prev->next = task->next;
	} else {
		readyLists[task->priority] = task->next; // change the head of the list
	}

	if (task->next) {
		task->next->prev = task->prev;
	}

	if (readyLists[task->priority] == NULL) { // head deleted, no other ready tasks
		readyBitmap &= ~(1 << task->priority);
	}
}


// scheduler global functions for synchronization primitives

void schedulerYield() {
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void schedulerBlockCurrent(semaphore* sem) {
	removeReadyTask(currentTaskController);
	currentTaskController->currentState = BLOCKED;
	currentTaskController->waitingFor = sem;
}

int schedulerUnblockTask(semaphore* sem) {
	int maxPriority = -1;
	int unblockedTask = -1;

	for (int i = 0; i < tasksManaged; i++) {
		if (taskList[i]->currentState == BLOCKED &&
		   (int)taskList[i]->priority > maxPriority &&
		   taskList[i]->waitingFor == sem) {
			maxPriority = taskList[i]->priority;
			unblockedTask = i;
		}
	}

	if (unblockedTask != -1) {
		taskList[unblockedTask]->currentState = READY;
		taskList[unblockedTask]->waitingFor = NULL;
		addReadyTask(taskList[unblockedTask]);

		if (taskList[unblockedTask]->priority > currentTaskController->priority) {
			return 1;
		}
	}

	return 0;
}

void idleTask(void) {
	while (1) {
		__WFI(); // low power wait for interrupt
	}
}

taskController* addTask(taskPtr taskProvided, uint8_t taskPriority) {
	uint32_t savedPrimask = __get_PRIMASK();
	__disable_irq();

	if (tasksManaged < MAX_TASKS) {
		// initialize a new task controller and stack top for the new task, so we can schedule it
		taskController*  newTaskController = &tcbStorage[tasksManaged];
		uint32_t* stackTop = &(taskStacks[tasksManaged][STACK_SIZE]);

		// initialize the stack frame for each task
		// hardware saves xPSR, PC, LR, R12, R3, R2, R1, R0
		*(--stackTop) = (1U << 24); // set ARM to thumb mode, and leave xPSR unchanged
		*(--stackTop) = (uint32_t)taskProvided; // set PC to return address of function
		*(--stackTop) = 0xFFFFFFFD; // set Link Register to 0xFFFFFFF 1101 (thread mode on, PSP for tasks, 0 enabled, and secure ARM boot)
		*(--stackTop) = 0x00000000; // R12
		*(--stackTop) = 0x00000000; // R3
		*(--stackTop) = 0x00000000; // R2
		*(--stackTop) = 0x00000000; // R1
		*(--stackTop) = 0x00000000; // R0

		// save into stack frame R11, R10, R9, R8, R7, R6, R5, R4
		*(--stackTop) = 0x00000000; // R11
		*(--stackTop) = 0x00000000; // R10
		*(--stackTop) = 0x00000000; // R9
		*(--stackTop) = 0x00000000; // R8
		*(--stackTop) = 0x00000000; // R7
		*(--stackTop) = 0x00000000; // R6
		*(--stackTop) = 0x00000000; // R5
		*(--stackTop) = 0x00000000; // R4

		// stack frame completed, pass the top of the stack pointer to the
		 newTaskController->taskStackPtr = stackTop;
		 newTaskController->priority = taskPriority;
		 newTaskController->currentState = READY;
		 newTaskController->waitingFor = NULL; // not waiting for anything

		 newTaskController->next = NULL;
		 newTaskController->prev = NULL;

		 taskList[tasksManaged] = newTaskController;
		 tasksManaged++;

		 if (taskProvided != idleTask) {
			 addReadyTask(newTaskController);
		 }
		 __set_PRIMASK(savedPrimask);
		 return newTaskController;
	}
	__set_PRIMASK(savedPrimask);
	return NULL;
}

void startScheduler(void) {

	idleTaskController = addTask(idleTask, 0);
	currentTaskController = idleTaskController;

	__set_PSP((uint32_t)idleTaskController->taskStackPtr);

	uint32_t pspMask = __get_CONTROL();
	pspMask |= (0b10); // turning on PSP is done by setting second least bit within CONTROL to be high
	__set_CONTROL(pspMask);
	__ISB(); // flush pipeline so we directly use PSP now

	// there are four priority bits on STM32
	NVIC_SetPriority(PendSV_IRQn, 0xF0);
	NVIC_SetPriority(SysTick_IRQn, 0xD0);
	SysTick_Config(SystemCoreClock / CLK_TICK_SPEED);

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // start the first task IMMEDIATELY so we don't have to wait a cycle

	while (1) __WFI();
}

void SysTick_Handler(void) {
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // force interrupt every tick
}

void scheduleHelper(void) {
	if (currentTaskController->currentState == READY) {
		addReadyTask(currentTaskController);
	}

	if (readyBitmap == 0) {
		currentTaskController = idleTaskController;
		return;
	}

	uint8_t maxPriority = 31 - __builtin_clz(readyBitmap); // count leading zeroes

	currentTaskController = readyLists[maxPriority];
	removeReadyTask(currentTaskController);
}

__attribute__((naked)) void PendSV_Handler (void) {
	__asm volatile(
			"CPSID I \n"

			"MRS R0, PSP \n"
			"STMDB R0!, {R4-R11} \n"

			"LDR R1, =currentTaskController \n" // load address of current task pointer
			"LDR R1, [R1] \n" // get the address of the actual task from the pointer
			"STR R0, [R1] \n" // push the new PSP into the first field of the currentTaskController

			"PUSH {LR} \n"
			"BL scheduleHelper \n"
			"POP {LR} \n"


			"LDR R1, =currentTaskController \n"
			"LDR R1, [R1] \n"
			"LDR R0, [R1] \n"

			"LDMIA R0!, {R4-R11} \n"
			"MSR PSP, R0 \n"

			"CPSIE I \n"
			"BX LR \n"
		);
}
