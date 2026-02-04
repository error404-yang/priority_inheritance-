// user/pi_simple.c
// Simple, clear priority inheritance demonstration

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("\n");
  printf("==============================================\n");
  printf("  Priority Inheritance Demonstration\n");
  printf("==============================================\n\n");
  
  printf("Scenario:\n");
  printf("  Process LOW (priority 10) will hold a lock\n");
  printf("  Process MED (priority 5) will do CPU work\n");
  printf("  Process HIGH (priority 1) will wait for lock\n");
  printf("  -> LOW should be boosted to priority 1\n\n");
  
  printf("Starting test...\n\n");
  
  // Create LOW priority process
  int pid = fork();
  if(pid == 0) {
    setpriority(10);
    printf("[LOW-10] Acquiring lock...\n");
    test_acquire();
    printf("[LOW-10] Got lock! Doing long work...\n");
    
    cpu_work(100000000);
    
    printf("[LOW-10] Done! Releasing lock...\n");
    test_release();
    exit(0);
  }
  
  // Wait a bit for LOW to get the lock
  for(int i = 0; i < 10000000; i++);
  
  // Create MEDIUM priority process
  pid = fork();
  if(pid == 0) {
    setpriority(5);
    printf("[MED-5] Doing CPU work (no lock needed)...\n");
    
    cpu_work(50000000);
    
    printf("[MED-5] Done!\n");
    exit(0);
  }
  
  // Wait a bit for MEDIUM to start
  for(int i = 0; i < 5000000; i++);
  
  // Create HIGH priority process
  pid = fork();
  if(pid == 0) {
    setpriority(1);
    printf("[HIGH-1] Need the lock! Waiting...\n");
    printf("[HIGH-1] >>> Priority inheritance should happen now! <<<\n");
    
    test_acquire();
    
    printf("[HIGH-1] Got lock!\n");
    test_release();
    exit(0);
  }
  
  // Wait for all processes
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n==============================================\n");
  printf("  Test Complete!\n");
  printf("==============================================\n");
  printf("\nLook for:\n");
  printf("  [PI] BOOST: ... (pri 10->1)  <- This means it worked!\n");
  printf("  [PI] RESTORE: ... (pri 1->10)\n\n");
  
  exit(0);
}
