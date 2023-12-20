## Assignment3


assignment3.c needs to be compiled using the attached Makefile, and it should be executed with sudo privileges.


1. assignment3.c : Source code that applies scheduling policies, including Default CFS, CFS with modified Nice values, RT_FIFO, and RT_RR, to child processes performing array operations. The code performs scheduling for these processes.

2. fifo.c : Source code for an additional assignment test.

3. Makefile : For compile the assignment3.c.

4. init_task.c : Modifies the task_struct of the top-level process, the init process, to change the default process scheduler policy in Linux.

location : /usr/src/linux/linux-5.15.120/init/

5. kthread.c : Changes the default scheduler policy for kernel threads.

location : /usr/src/linux/linux-5.15.120/kernel/

6. exit.c : Outputs the CPU burst of a terminated process to the kernel log.

location : /usr/src/linux/linux-5.15.120/kernel/


## Basic Assignment

For CFS_DEFAULT, fork is performed without any value changes. For CFS_NICE, the scheduler policy remains unchanged, and only the nice values are set to 19, 0, -20 for 7 processes. Afterward, a system call is made to modify the scheduler. For RT_FIFO, the scheduler policy and rt_priority are changed, followed by a system call to adjust the scheduler. For RT_RR, the scheduler policy and time quantum are modified to 10ms, 100ms, and 1000ms, respectively. A system call is then made to update the scheduler.


## Additional Assignment

To change the Linux Default Scheduler from CFS to RT_FIFO, the task_struct of the init process, defined in init_task, had its policy and priority updated to the priority range of SCHED_FIFO and RT_FIFO.

In Linux, all processes are forked from the init process created during boot. Hence, all child processes under the init process inherit the scheduler policy from the root init process unless explicitly changed by system calls or specific conditions. Therefore, the policy and priority of the init process, defined in init_task, were modified.

Additionally, to ensure that kernel threads managed by kthread are also scheduled as RT_FIFO, the __kthread_create_on_node function, responsible for the creation of kernel threads, was modified to invoke sched_setscheduler_nocheck with SCHED_FIFO.
