# aki-rtos
_**High-performance RTOS designed for STM32 MCUs, with preemptive O(1) task scheduling, message queues, priority inheritance. <br> Achieved 4.309µs context switching, 11.309µs scheduler boot time, 16.44µs task creation to execution.**_

Aki-RTOS is a bare-metal lightweight RTOS designed for embedded systems (primarily STM32, or any microcontroller with ARM Architecture) and can run on multi-core systems. It provides efficient preemptive task scheduling with priority inheritance, lightweight inter-task communication, and robust interrupt handling. Synchronization primitives include mutexes and semaphores. Aki-RTOS also supports optional ISR-to-Task communication, with robust ISR non-blocking message queue functions, and non-blocking semaphore access.
