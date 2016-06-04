#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/cred.h>

//Process info struct to hold info about our current process
struct processinfo {   // struct processinfo
    long state;//Current state of process
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

//system call table
unsigned long **sys_call_table;

//reference to original system call to intercept
asmlinkage long (*ref_sys_cs3013_syscall2)(struct processinfo *info);

//function to replace original call
asmlinkage long new_cs3013_syscall2(struct  processinfo *kinfo)
{
  struct list_head *child;
  struct list_head *sib;
  int youngestChildPid = -1;
  int nextOlderSibPid = -1;
  int nextYoungerSibPid = -1;
  struct timespec youngestTime  = {0 , 0};
  struct timespec youngestSibTime  = {0 , 0};
  struct timespec oldestSibTime  = {0xFFFFFFFF , 0xFFFF};
  struct timespec *youngest;
  struct timespec *nextOlder;
  struct timespec *nextYounger;
  int ppid;
  youngest = &youngestTime;
  nextOlder = &oldestSibTime;
  nextYounger = &youngestSibTime;

  //Get Parent PID
  rcu_read_lock();
  ppid = task_tgid_vnr(rcu_dereference(current->real_parent));
  rcu_read_unlock();
  ////////////////
  
  //iterate list of children to find youngest child
  list_for_each(child, (&current->children))
  {

    struct task_struct *entry = list_entry(child, struct task_struct, children);

    if ((timespec_compare(&entry->start_time, youngest) >0) && entry != NULL)
    {
      youngest = &entry->start_time;
      youngestChildPid = entry->pid;
    }
  }
  
  //iterate sibling list to find younger sibling and older sibling
  list_for_each(sib, (&current->sibling))
  {

    struct task_struct *entry = list_entry(sib, struct task_struct, sibling);
    if(list_empty(&current->sibling) != 0)//If the list is empty
    {

    }
    else
    {
      if ((timespec_compare(&entry->start_time, nextOlder) < 0) && (entry != NULL) && (timespec_compare(&entry->start_time , &current->start_time)) > 0)
      {
        nextOlder = &entry->start_time;
        nextOlderSibPid = entry->pid;
      }

      if ((timespec_compare(&entry->start_time, nextYounger) > 0) && entry != NULL && (timespec_compare(&entry->start_time , &current->start_time)) < 0)
      {
        nextYounger = &entry->start_time;
        nextYoungerSibPid =  entry->pid;
      }
      
    }
  }

  //printing to syslog
  printk("------------------------------------\n");
  printk("Current State: %ld\n", current->state);
  printk("Current PID: %d\n", current->pid);
  printk("Parent PID: %d\n", ppid);
  printk("Youngest child: %d\n", youngestChildPid);/////////////////////@TODO
  printk("Younger sibling: %d\n", nextYoungerSibPid);/////////////////////@TODO
  printk("Older sibling: %d\n", nextOlderSibPid);/////////////////////@TODO
  printk("User ID (of process owner): %d\n", from_kuid_munged(current_user_ns(), current_uid()));
  printk("Start time (nanos) since boot: %llu\n", 1000*((unsigned long long)(current->real_start_time.tv_sec * 1000*1000*1000) + (unsigned long long)current->real_start_time.tv_nsec));
  printk("CPU time in user mode (micros): %llu\n", (long long unsigned int)cputime_to_usecs(current->utime));
  printk("CPU time in system mode (micros): %llu\n", (long long unsigned int)cputime_to_usecs(current->stime));
  printk("User time of children (micros): %llu\n", (long long unsigned int)cputime_to_usecs(current->signal->cutime) - (long long unsigned int)cputime_to_usecs(current->utime) );
  printk("System time of children (micros): %llu\n", (long long unsigned int)cputime_to_usecs(current->signal->cstime) - (long long unsigned int)cputime_to_usecs(current->stime) );
  
  return 0;//Gets the current PID.
}

//finds system table. this function was provided
static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
                (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
   */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_cs3013_syscall2;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  //sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
  //sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);