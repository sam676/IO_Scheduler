# IO_Scheduler
Operating Systems Project - simulate an I/O Scheduler

In this lab I simulated the scheduling of IO operations. 

Applications submit their IO requests to the IO subsystem, where they are maintained in an IO-queue until the device is ready for issuing another request. 
The IO-scheduler then selects a request from the IO-queue and submits it to the disk, aka the strategy() routine in operating systems. 
On completion another request can be taken from the IO-queue and submitted to the disk. The scheduling policies will allow for some optimization as to reduce disk head movement or overall wait time in the system. 

The schedulers to be implemented are FIFO (i), SSTF (j), LOOK (s), CLOOK (c), and FLOOK (f) (the letters in bracket define which parameter must be given in the –s program flag).

Invocation is a follows:
./iosched –s<schedalgo> <inputfile>
  
The input file is structured as follows: Lines starting with ‘#’ are comment lines and should be ignored.
Any other line describes an IO operation where the 1st integer is the time step at which the IO operation is issued and the 2nd integer is the track that is accesses. Since IO operation latencies are largely dictated by seek delay (i.e. moving the head to the correct track), rotational and transfer delays are ignored for simplicity.

Moving the head by one track will cost one time unit writen as integers. The disk can only consume/process one IO request at a time. Everything else is maintained in an IO queue and managed according to the scheduling policy. The initial direction of the LOOK algorithms is from 0-tracks to higher tracks. The head is initially positioned at track=0 at time=0. Note that the maxtrack (think SCAN vs. LOOK) does not have to be known.

Each simulation prints information on individual IO requests followed by a SUM line that has computed some statistics of the overall run. 
