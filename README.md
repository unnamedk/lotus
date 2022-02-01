# lotus
Kernel-mode driver for Windows that performs memory functions. Made for learning windows internals/reverse engineering

## Features

* Pattern scanning
* Clearing loaded driver history of MmUnloadedDrivers and PidDBCacheTable to be more stealthy
* Communication using shared memory that can also be opened by user-mode processes
* Stealth thread creation for communication purposes (unlink from official thread list)
* Read/write memory on-demand
* Allocate/free memory in any process
* Allocate memory on kernel space
* Mark pages as executables
