# Producer-Consumer

This assignment is involved the mutex and circular shared memory buffer. The producer is going to read in the file and put the file into the shared memory buffer as the first process, and the consumer as second process is going to read the file from the shared memory buffer and swap each pair in the file.

I used Mutex in this assignment to avoid any deadlock or race condition. The Mutex is going to lock the shared memory while it transfers file between the producer and buffer or from buffer to consumer.

I also used fork() for the extra-credit . Basically fork() is going to auto create the processes instead us doing it manually in 2 different terminals. fork() creates the parent (producer) and the child (consumer) for us. 

Overall , this assignment was a really good exercise for me to learn how to provide a solution to the producer-consumer problem using fork(), Mutex, condition variables to allow efficient data processing using a circular buffer that is created through a shred memory.
