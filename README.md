# Aki-RTOS
_**High-performance RTOS designed for STM32 MCUs, with preemptive O(1) task scheduling, message queues, priority inheritance. <br> Achieved 4.309µs context switching, 11.309µs scheduler boot time, 16.44µs task creation to execution.**_

Aki-RTOS is a bare-metal lightweight RTOS designed for embedded systems (primarily STM32, or any microcontroller with ARM Architecture) and can run on multi-core systems. It provides efficient preemptive task scheduling with priority inheritance, lightweight inter-task communication, and robust interrupt handling. Synchronization primitives include mutexes and semaphores. Aki-RTOS also supports optional ISR-to-Task communication, with robust ISR non-blocking message queue functions, and non-blocking semaphore access.

## Features
- O(1) Task Scheduling with Ready Queues
- Message Queues for Task-Task or ISR-Task Communication
- Mutexes with Priority Inheritance, Semaphores
- PendSV Context Switching, SysTick Preemptive Interrupts
- Anti-Starvation for Same-Priority Tasks

## How to Use
### Adding Tasks & Booting Scheduler
Implement tasks (functions) without parameters. Parameters can be passed by message queues, or by statically storing them at some memory address for the task to access.
```addTask(taskPtr functionName, priority);```
Priority ranges from 0 to 10, with 10 being highest importance, and 0 being lowest importance. Tasks are scheduled within ready queues, for instant O(1) selection. Tasks of the same priority are rotated between during context switches to prevent starvation.
```startScheduler();```
Begins the scheduler, and begins to perform tasks. Tasks can use the `taskFinish()` function to stop execution. Utilizing this function will prevent execution ever again, but won't delete the allocated memory to the TCB.

### Semaphores
#### Initialization
```semaphoreConstructor(semaphore* sem, uint8_t resources)```
All semaphores must be initialized by this function, which sets the number of resources currently available for a semaphore.

#### Semaphore Task Functions
```semaphoreBlock(semaphore* sem)```
Tasks can call this to initiate a blocking wait for a semaphore. If there are resources available right now, a resource is subtracted from the semaphore, and the task continues execution. Otherwise, the task is put to sleep and blocked until another task posts resources.
```int semaphorePost(semaphore* sem)```
Tasks can call this to return or add a resource to the semaphore. The semaphore then unblocks the highest priority task waiting on the semaphore, and immediately triggers a context switch if the unblocked tasks's priority is higher than the posting task.

#### Semaphore ISR Functions
```int semaphoreTry(semaphore* sem)```
ISRs can call this to attempt to access a resource held by a semaphore. If resources are available, the semaphore subtracts a resource and the function returns 1 for success. Otherwise, no resources are deducted, and the function returns 0 for failure.
```void semaphorePostISR(semaphore* sem)```
ISRs can call this to add a resource back to a semaphore. The highest priority task waiting on the semaphore is unblocked, but it is not run.

### Mutexes
#### Initialization
```mutexConstructor(mutex* mutex)```
All mutexes must be initialized by this function, creating a mutex with no current owner and one available resource.

#### Locking & Unlocking (Priority Inheritance)
```
mutexLock(mutex* mutex, taskController* tcb)
```
Tasks can pass their TCB (currentTaskControlller) and the mutex they want to own. If no task currently holds the mutex, the passed TCB immediately gains ownership of the mutex. If a task is holding the mutex, the task priority of the current owner is upgraded to the calling task's priority, if higher. The calling task then sleeps until it's the highest priority waiting task and the mutex is released. On wake, it immediately gains mutex ownership.

```
mutexUnlock(mutex* mutex, taskController* tcb)
```
Tasks can release the mutex if they currently hold it by calling this function. If the calling task doesn't hold the mutex, or the mutex is ownerless, nothing occurs. Otherwise, the calling task's priority is reset (in case of priority inheritance) and the task releases the mutex.

### Message Queues
#### Initialization
```
messageQueue queue;
type dataBuffer[QUEUE_SIZE]
```
For queues, buffers must be externally maintained, so different size message queues can be implemented. When initializing a message queue, an array of the data type within the queue must also be initialized.
```queueConstructor(messageQueue* queue, void* dataLocation, uint8_t typeSize, uint8_t queueCapacity)```
All message queues must be initialized by this function, creating a message queue pointing to the externally-maintained array. Message queues are single-port, meaning only one core can be reading or writing at any given time.

#### Task-to-Task Communication
```queueSend(messageQueue* queue, const void* item)```
If capacity permits in the queue, decrements the number of queue spaces available, copies the item into the messageQueue, and posts a signal that an item is available in the queue. Otherwise, if the queue is full, the task sleeps until spaces are freed. **Cannot be interrupted midway by an ISR, and other threads cannot read or write during this.**
```queueReceive(messageQueue* queue, void* destination)```
If data exists in the queue, decrements the number of items available, copies the item into destination, and posts a signal that a new space is available in the queue. Otherwise, if the queue is empty, the task sleeps until an item is added to the queue. **Cannot be interrupted midway by an ISR, and other threads cannot read or write during this.**

#### ISR-to-Task Communication
```int queueSendISR(messageQueue* queue, const void* item)```
If capacity permits in the queue, decrements the number of queue spaces available, copies the item into the messageQueue, and posts a signal that an item is available in the queue, returning 1 for success. Otherwise, if the queue is full, returns 0 for failure. **Cannot be interrupted midway by an higher priority ISR.**
```queueReceiveISR(messageQueue* queue, void* destination)```
If data exists in the queue, decrements the number of items available, copies the item into destination, and posts a signal that a new space is available in the queue, returning 1 for success. Otherwise, if the queue is empty, the task sleeps until an item is added to the queue. **Cannot be interrupted midway by an higher priority ISR.**

#### Additional Message Queue Functions
```int queueEmpty(messageQueue* queue)```
Returns 1 if the queue is empty, and 0 otherwise. **Other tasks may be reading or writing during this.**
```int queueFull(messageQueue* queue)```
Returns 1 if the queue is full, and 0 otherwise. **Other tasks may be reading or writing during this.**
```queueReset(messageQueue* queue)```
Resets the message queue, setting queue capacity to full. **Cannot be interrupted midway by an ISR, and OVERWRITES the mutex. Use only when no other task/core is accessing the queue.**
```queuePeek(messageQueue* queue)```
If items available in the queue, directly grabs the first item available without removing it from the queue. **Cannot be interrupted by an ISR, but other tasks may read or write during this.**


## Benchmarking

## Acknowledgements
