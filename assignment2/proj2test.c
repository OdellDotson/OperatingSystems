#include <sys/syscall.h>
#include <stdio.h>

// These values MUST match the syscall_32.tbl modifications:
#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

long testCall1 ( void) {
        return (long) syscall(__NR_cs3013_syscall1);
}
long testCall2 ( void) {
        return (long) syscall(__NR_cs3013_syscall2);
}
long testCall3 ( void) {
        return (long) syscall(__NR_cs3013_syscall3);
}
int main () {
        printf("The return values of the system calls are:\n");
        printf("\tcs3013_syscall2: %ld\n", testCall2());
        return 0;
}
