## Assignment2


assignment2.c : Source code implementing reverse display of digits and sign conversion operations.


syscall_64.tbl : For a 64-bit Ubuntu operating system installed on a virtual machine, registering a new system call in the 64-bit system call table.

location : /usr/src/linux/linux-5.15.120/arch/x86/entry/syscalls/


syscalls.h : Defining the prototype of the new system call in the system call header file.

location : /usr/src/linux/linux-5.15.120/include/linux/


sys_reverse_print.c : New system call source code that prints the digits of an input stream in reverse order.

location : /usr/src/linux/linux-5.15.120/kernel/


sys_compute_plus.c : New system call source code that performs addition operation on two input values.

location : /usr/src/linux/linux-5.15.120/kernel/


sys_compute_minus.c : New system call source code that performs subtraction operation on two input values.

location : /usr/src/linux/linux-5.15.120/kernel/


Makefile : Makefile for compiling the modified or added kernel source code.

location : /usr/src/linux/linux-5.15.120/kernel/
