# aki-rtos
_**High-performance RTOS designed for STM32 MCUs, with preemptive O(1) task scheduling, message queues, priority inheritance. <br> Achieved 4.309µs context switching, 11.309µs scheduler boot time, 16.44µs task creation to execution.**_

Aki-RTOS is a bare-metal lightweight RTOS designed for embedded systems (primarily STM32, or any microcontroller with ARM Architecture). Provides efficient preemptive task scheduling with priority inheritance, lightweight inter-task communication, and robust interrupt handling. Mutexes and semaphores both implemented as synchronization primitives, and robustly implemented to be capable of running on a multi-core system.
