#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
busywait(int n)
{
  // Simple busy wait
  for(volatile int i = 0; i < n * 1000000; i++)
    ;
}

int
main(int argc, char *argv[])
{
  printf("=== SIMPLE SCHEDULER TEST ===\n");
  
  int pid1 = fork();
  if(pid1 < 0) {
    printf("ERROR: fork failed\n");
    exit(1);
  }
  
  if(pid1 == 0) {
    printf("CHILD1: Started\n");
    for(int i = 0; i < 3; i++) {
      printf("CHILD1: iteration %d\n", i);
      busywait(10);
    }
    printf("CHILD1: Exiting\n");
    exit(0);
  }
  
  busywait(20);
  
  int pid2 = fork();
  if(pid2 < 0) {
    printf("ERROR: fork failed\n");
    exit(1);
  }
  
  if(pid2 == 0) {
    printf("CHILD2: Started\n");
    for(int i = 0; i < 3; i++) {
      printf("CHILD2: iteration %d\n", i);
      busywait(10);
    }
    printf("CHILD2: Exiting\n");
    exit(0);
  }
  
  printf("PARENT: Waiting for children...\n");
  wait(0);
  printf("PARENT: First child done\n");
  wait(0);
  printf("PARENT: Second child done\n");
  printf("PARENT: Test complete!\n");
  
  exit(0);
}
