// user/pi_simple_demo.c
// Simplified priority inheritance demo - very clear output
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void work(long n) {
  volatile long s = 0;
  for(long i = 0; i < n; i++) s += i;
}

int main(void)
{
  printf("\n========================================\n");
  printf("  Priority Inheritance - Simple Demo\n");
  printf("========================================\n\n");

  // LOW priority process
  if(fork() == 0) {
    setpriority(10);
    printf("LOW (pri=10): Acquiring lock...\n");
    work(5000000);
    
    test_acquire();
    printf("LOW (pri=10): Got lock! Working...\n");
    work(5000000);
    
    // Work in chunks and show priority
    for(int i = 1; i <= 4; i++) {
      int p = getpriority();
      printf("LOW: Working... (pri=%d)\n", p);
      work(10000000);
    }
    
    int final_pri = getpriority();
    printf("LOW (pri=%d): Releasing lock\n", final_pri);
    test_release();
    
    int restored_pri = getpriority();
    printf("LOW (pri=%d): Done!\n", restored_pri);
    exit(0);
  }

  // Let LOW acquire lock first
  work(20000000);

  // HIGH priority process
  if(fork() == 0) {
    setpriority(1);
    printf("\n>>> HIGH (pri=1): Trying to get lock...\n");
    printf(">>> This should BOOST LOW to pri=1!\n\n");
    work(5000000);
    
    test_acquire();
    printf("HIGH (pri=1): Got lock!\n");
    test_release();
    printf("HIGH (pri=1): Done!\n");
    exit(0);
  }

  wait(0);
  wait(0);

  printf("\n========================================\n");
  printf("  If you see LOW's pri change 10->1->10,\n");
  printf("  priority inheritance is WORKING!\n");
  printf("========================================\n\n");
  
  exit(0);
}
