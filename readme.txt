# Build Instructions:
make
By using the original Makefile (with gcc command included) in the repo, the instruction "make" (under the repo csc415-p5-jzhao11/part2) can build the program and generate the executable file "pandc".


# Run Instructions:
./pandc N P C X Ptime Ctime
This instruction is also executed under csc415-p5-jzhao11/part2, which runs the executable file "pandc" with 6 parameters. Descriptions about these parameters are listed as follows.
N: number of buffers
P: number of producer threads
C: number of consumer threads
X: number of items produced by each producer thread
Ptime: time each producer sleeps (in seconds)
Ctime: time each consumer sleeps (in seconds)


# Project Description:
A global atomic counter is used to generate numbers for producer threads. To guarantee synchronization, the semaphore mutex will work as a mutex lock, where each thread function will acquire it before entering the critical section (accessing the global shared resource) and release it after exiting the critical section.

The main function will initialize the bounded buffer and related parameters using the command line arguments. Then it will create producer and consumer threads, with each of them calling corresponding thread function. The producer function will repeatedly produce unique numbers (with info printed on terminal) by using the global counter, and enqueue the integers to the bounded buffer. By contrast, the consumer function will repeatedly dequeue integers stored in the buffer, and consume it (with info printed). Semaphores full and empty are working together to ensure that each produced item will be enqueued only when the buffer is not full, and that each consumed item will be dequeued only when the buffer is not empty. At last, all threads will join the main process and the main() will finalize.

Each item produced by a producer thread will be collected in a producer array, while each item consumed by a consumer will also be stored in a consumer array. These two arrays will be compared and printed to check whether a series of unique numbers are produced, and whether they are consumed in the same order. If producer and consumer arrays match and those unique numbers are correctly ordered, it proves that the producers and consumers are executed in a correct sequence.
