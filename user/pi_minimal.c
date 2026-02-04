// user/pi_minimal.c
// Minimal test - reports priority changes from USER SPACE

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;

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
  printf("\n=== MINIMAL Priority Inheritance Test ===\n\n");
  
  stage = 0;
  
  // LOW priority
  if(fork() == 0) {
    int pid = getpid();
    setpriority(10);
    
    printf("[PID=%d][LOW] Pri=%d, acquiring lock\n", pid, getpriority());
    test_acquire();
    stage = 1;
    
    printf("[PID=%d][LOW] Got lock, pri=%d\n", pid, getpriority());
    
    // Check priority periodically during work
    for(int chunk = 0; chunk < 5; chunk++) {
      user_work(30000000);
      printf("[PID=%d][LOW] Chunk %d, pri=%d\n", pid, chunk, getpriority());
    }
    
    printf("[PID=%d][LOW] Releasing, pri=%d\n", pid, getpriority());
    test_release();
    printf("[PID=%d][LOW] Released, pri=%d\n", pid, getpriority());
    exit(0);
  }
  
  while(stage < 1);
  for(int i = 0; i < 10000000; i++);
  
  // MEDIUM priority
  if(fork() == 0) {
    int pid = getpid();
    setpriority(5);
    printf("[PID=%d][MED] Pri=%d, working\n", pid, getpriority());
    stage = 2;
    user_work(80000000);
    printf("[PID=%d][MED] Done\n", pid);
    exit(0);
  }
  
  while(stage < 2);
  for(int i = 0; i < 15000000; i++);
  
  // HIGH priority
  if(fork() == 0) {
    int pid = getpid();
    setpriority(1);
    printf("\n[PID=%d][HIGH] Pri=%d, acquiring lock\n", pid, getpriority());
    printf("[PID=%d][HIGH] >>> BOOST should happen! <<<\n\n", pid);
    
    test_acquire();
    
    printf("\n[PID=%d][HIGH] Got lock!\n", pid);
    test_release();
    exit(0);
  }
  
  wait(0); wait(0); wait(0);
  
  printf("\n=== Test Complete ===\n");
  printf("Look for:\n");
  printf("  - LOW's priority changing from 10 to 1\n");
  printf("  - [PI] restore message\n\n");
  
  exit(0);
}
