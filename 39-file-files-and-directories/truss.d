#!/usr/sbin/dtrace -s

/* run this program while passing in the pid like so: */
/* dtrace -q -s trussrw.d 12345 */
/* if the pid is that of a shell (obtained via `echo $$`), then you can run arbitrary commands and see the trace */
/* syscall::open:entry, */
/* syscall::read:entry, */
syscall::write:entry
/* syscall::close:entry */
/pid == $1/
{
   /* The first argument to read(2) and write(2) is a file descriptor, printed in decimal.
      The second argument is a buffer address, formatted as a hexadecimal value.
      The final argument is the buffer size, formatted as a decimal value. */
	/* printf("%s(%d, 0x%x, %4d)\n", probefunc, arg0, arg1, arg2); */
        printf("(%d) %s %s", pid, probefunc, copyinstr(arg1));
}

syscall::open:entry,
syscall::read:entry,
syscall::write:entry,
syscall::close:entry
/pid == $1/
{
  /* Notice that the syscall provider also publishes a probe named return for each system call in addition to entry.
     The DTrace variable arg1 for the syscall return probes evaluates to the system call's return value */
	printf("\t\t = %d\n", arg1);
}


