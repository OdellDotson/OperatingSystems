#include <sys/syscall.h>
#include <stdio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/cred.h>

// These values MUST match the syscall_32.tbl modifications:
#define __NR_cs3013_syscall2 356

struct processinfo {   // struct processinfo
    long state;
    pid_t pid;// process ID of this process
    pid_t parent_pid;// process ID of parent
    pid_t youngest_child;// process ID of youngest child
    pid_t younger_sibling;// pid of next younger sibling
    pid_t older_sibling;// pid of next older sibling
    uid_t uid;// user ID of process owner
    long long start_time;// process  time in nanoseconds since boot time
    long long user_time;// CPU time in user mode (microseconds) 
    long long sys_time;// CPU time in system mode (microseconds) 
    long long cutime;// user time of children (microseconds)
    long long cstime;// system time of children (microseconds)
};

long testCall2 (struct  processinfo *info) {
        return (long) syscall(__NR_cs3013_syscall2);
}

int main () {

        printf("\tcs3013_syscall2: %ld\n", testCall2(processinfo));
        return 0;
}
