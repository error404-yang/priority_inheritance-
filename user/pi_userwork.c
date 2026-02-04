// user/pi_userwork.c
// Safe test - does CPU work in USER SPACE, not kernel syscall

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;

// Do CPU work in USER SPACE - not a syscall!
void
user_work(int iterations)
{
  volatile long sum = 0;
  for(long i = 0; i < iterations; i++) {
    sum += i;
  }
}

int
main(void)
{
  printf("\n");
  printf("==================================================\n");
  printf("  WORKING Priority Inheritance Test\n");
  printf("  (CPU work done in user space)\n");
  printf("==================================================\n\n");
  
  stage = 0;
  
  // LOW priority process
  if(fork() == 0) {
    setpriority(10);
    int pid = getpid();
    
    printf("[PID=%d][LOW-10] Acquiring lock\n", pid);
    test_acquire();
    
    stage = 1;
    printf("[PID=%d][LOW-10] Lock acquired, priority=%d\n", pid, getpriority());
    printf("[PID=%d][LOW-10] Doing work...\n", pid);
    
    // Do work in USER SPACE - no syscall!
    user_work(150000000);
    
    printf("[PID=%d][LOW-10] Work done, priority=%d\n", pid, getpriority());
    printf("[PID=%d][LOW-10] Releasing lock\n", pid);
    
    test_release();
    
    printf("[PID=%d][LOW-10] Released, priority=%d\n", pid, getpriority());
    exit(0);
  }
  
  // Wait for LOW to get lock
  while(stage < 1);
  for(int i = 0; i < 10000000; i++);
  
  // MEDIUM priority process  
  if(fork() == 0) {
    setpriority(5);
    int pid = getpid();
    
    printf("[PID=%d][MED-5] Doing work (no lock)\n", pid);
    stage = 2;
    
    user_work(80000000);
    
    printf("[PID=%d][MED-5] Done\n", pid);
    exit(0);
  }
  
  // Wait for MEDIUM
  while(stage < 2);
  for(int i = 0; i < 15000000; i++);
  
  // HIGH priority process
  if(fork() == 0) {
    setpriority(1);
    int pid = getpid();
    
    printf("\n[PID=%d][HIGH-1] Acquiring lock\n", pid);
    printf("[PID=%d][HIGH-1] *** Should boost LOW ***\n\n", pid);
    
    test_acquire();
    
    printf("\n[PID=%d][HIGH-1] Got lock!\n", pid);
    
    test_release();
    
    printf("[PID=%d][HIGH-1] Done\n", pid);
    exit(0);
  }
  
  // Wait for all
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n==================================================\n");
  printf("  Test Complete - NO PANIC!\n");
  printf("==================================================\n\n");
  
  exit(0);
}
