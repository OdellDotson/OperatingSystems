#include <sys/syscall.h>
#include <stdio.h>

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

long testCall1 (void *) {
        return (long) syscall(__NR_cs3013_syscall1);
}

int main () {

        printf("\tcs3013_syscall1: %ld\n", testCall1());
        return 0;
}
