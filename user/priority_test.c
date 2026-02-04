#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("====================================\n");
  printf("  BASIC PRIORITY TEST\n");
  printf("====================================\n");
  
  int pid1 = fork();
  if(pid1 == 0) {
    // Child 1 - LOW priority
    setpriority(10);
    printf("[PID=%d] LOW priority (10) - starting\n", getpid());
    for(int i = 0; i < 5; i++) {
      printf("[PID=%d] LOW - count %d\n", getpid(), i);
      // Yield to allow other processes to run
      for(volatile int j = 0; j < 1000000; j++);
    }
    printf("[PID=%d] LOW - exiting\n", getpid());
    exit(0);
  }
  
  int pid2 = fork();
  if(pid2 == 0) {
    // Child 2 - HIGH priority
    setpriority(1);
    printf("[PID=%d] HIGH priority (1) - starting\n", getpid());
    for(int i = 0; i < 5; i++) {
      printf("[PID=%d] HIGH - count %d\n", getpid(), i);
      // Yield to allow other processes to run
      for(volatile int j = 0; j < 1000000; j++);
    }
    printf("[PID=%d] HIGH - exiting\n", getpid());
    exit(0);
  }
  
  // Parent waits for both
  printf("[PARENT] Waiting for children...\n");
  wait(0);
  wait(0);
  printf("[PARENT] Both children done\n");
  printf("====================================\n");
  printf("HIGH priority should have printed more!\n");
  printf("====================================\n");
  
  exit(0);
}
