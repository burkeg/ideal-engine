#include "semUtils.h"

sem_t* setup_sem(int value, char * name) {
  sem_t* sem;
  sem = sem_open (name, O_CREAT | O_EXCL, 0644, value);
  if (errno == EEXIST) { //checks to see if semaphore is still alive
    cleanup_sem(sem,name);
    printf("%s: not properly deallocated, reinitializing.\n",name);
    sem = sem_open(name, O_CREAT | O_EXCL, 0644, value);
  }
  return sem;
}

void cleanup_sem(sem_t* sem, char* name) {
  sem_unlink(name);
  /* unlink prevents the semaphore existing forever */
  /* if a crash occurs during the execution         */
  sem_close(sem);
  printf("Closed semaphore %s\n",name);
}

// https://stackoverflow.com/questions/6331301/implementing-an-n-process-barrier-using-semaphores
//To use, initialize semaphore to 0 and mutex to 1.
//Waits until barrier is reached by n processes, then releases all
void barrier(sem_t* sem,sem_t* mutex, int* counter, int n)
{
  sem_wait(mutex);
  (*counter)++;
  sem_post(mutex);
  if (*counter == n) {
    sem_post(sem);
  }  
  sem_wait(sem);
  sem_post(sem);
}
