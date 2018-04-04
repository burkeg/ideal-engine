#include "idealEngine.h"

/*
  https://stackoverflow.com/questions/16400820/c-how-to-use-posix-semaphores-on-forked-processes
*/
key_t shmkey;
int shmid;   
sem_t *fork_sem;
sem_t *mutex;
sem_t *sem_master_ready;
sem_t *sem_master_completed;
partition_bounds partition_starts[NUM_PARTITIONS_MAP];
struct timeval stop, start;

int main (int argc, char *argv[]) {
  int i;
  int* barrier_count;
  int *data;
  FILE *fp;
  int * buffer;
  int numEl;
  long size;    
  pid_t pid;
  char * lock_str = "lSem";
  char * fork_str = "fSem";
  char * master_str = "mSem";
  char * completed_str = "cSem";
  int myID;
  exe_name = argv[0];
  printf("FIRST PID:%d\n",getpid());
  
  /* initialize a shared variable in shared memory */
  shmkey = ftok ("/dev/null", 5);       /* valid directory name and a number */
  //  printf ("shmkey for barrier_count = %d\n", shmkey);
  shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
  if (shmid < 0){                           /* shared memory error check */
    perror ("shmget\n");
    exit (1);
  }

  barrier_count = (int *) shmat (shmid, NULL, 0);   /* attach p to shared memory */
  *barrier_count = 0;
  //  printf ("barrier_count=%d is allocated in shared memory.\n\n", *barrier_count);
  
  
  /* initialize semaphores for shared processes */
  fork_sem = setup_sem(0, fork_str);
  mutex = setup_sem(1, lock_str);
  sem_master_ready = setup_sem(0, master_str);
  sem_master_completed = setup_sem(0, completed_str);
  
  pid = fork();
  // printf("pid: %d",pid);
  
  gettimeofday(&start, NULL);
  if (pid!=0) {
    //Master
    /*
    
    fp = fopen("data.hex","r");
    if (fp==NULL) {
      printf("Cannot open file\n");
      return -1;
    }
  
    fseek (fp, 0, SEEK_END);
    size=ftell(fp);
    fseek( fp, 0, SEEK_SET );
    fread(&numEl,4,1,fp);
    // printf("%d\n",numEl);
  
    buffer = (int *)malloc(sizeof(int)*size);
    if (buffer==NULL) {
      printf("Cannot allocate buffer\n");
      return -1;
    }
    fread(buffer,4,numEl,fp);
    
    for (i = 0; i < numEl; i++) {
      //printf("%d\n",(int)buffer[i]);
    }
    //printf("%d\n",NUM_PARTITIONS_MAP);
    //mapping = (int *) mmap(startingArr,400,PROT_WRITE,);

    fseek(fp,4,SEEK_SET);
    *(partition_starts[0].start)=ftell(fp);
    for (i=1; i < NUM_PARTITIONS_MAP; i++) {
      fseek(fp,size/NUM_PARTITIONS_MAP,SEEK_CUR);
      *(partition_starts[i-1].end)=ftell(fp)-1;
      *(partition_starts[i].start)=ftell(fp);
    }
    fseek(fp,0,SEEK_END);
    *(partition_starts[NUM_PARTITIONS_MAP-1].end)=ftell(fp);
  
    for (i=0; i < NUM_PARTITIONS_MAP; i++) {
      //      printf("%d:(%ld,%ld)\n",i,partition_starts[i].start,partition_starts[i].end);
    }
    fclose(fp);
    */

    //    printf("\nJust Exiting...\n");
    //gettimeofday(&stop, NULL);
    //printf("BEFORE %lu\n", stop.tv_usec - start.tv_usec);
    sem_post(fork_sem);
    printf("Ready to start master\n");
    
    sem_wait(sem_master_ready);
    printf("Ready to start master for real\n");

    initMaster();
    
    printf("Done initializing master\n");
    sem_post(sem_master_completed);
    
    delegateTasks();
    
    printf("Done with master\n");
    sem_post(sem_master_completed);
    
    return 0;
    printf("PULLING HAIR OUT");
  }
  //  sem_wait(fork_sem);
  //printf("Should make through but not enter 2nd stage yet\n");
  sem_wait(fork_sem);
  //printf("\nEntered..\n");
  
  
  //gettimeofday(&stop, NULL);
  //printf("AFTER %lu\n", stop.tv_usec - start.tv_usec);
  // printf("\n--------------\n");

  //creates n workers
  myID=NUM_WORKERS;
  for (i = 0; i < NUM_WORKERS - 1; i++) {
    pid = fork ();
    if (pid < 0) {
      /* check for error */
      cleanup_sem(fork_sem,fork_str);
      printf ("Fork error.\n");
    }
    else if (pid == 0)
      break;             /* child processes */     
    myID=i+1;
    
  }
  
  printf("My ID,PID is %d,%d.\n",myID,getpid());


  // barrier(fork_sem,mutex,barrier_count,NUM_WORKERS-1);
  /*
  sem_wait(mutex);
  (*barrier_count)++;
  //  printf("Barrier: %d\n",*barrier_count);
  sem_post(mutex);
  if (*barrier_count == NUM_PARTITIONS_MAP + NUM_PARTITIONS_REDUCE-1) {
    sem_post(fork_sem);
  }  
  sem_wait(fork_sem);
  sem_post(fork_sem);
  */
  if (pid != 0) {
    printf("All worker processes made.\n");
    sem_post(sem_master_ready);
    //    sem_wait(sem_master_completed);
  }
  printf("%d before\n",getpid());
  barrier(fork_sem,mutex,barrier_count,NUM_WORKERS);
  printf("%d after\n",getpid());
  printf("Got past worker barrier %d.\n",getpid());
  work(myID);
  
  

  //Cleanup after finished
  
  if (pid != 0) {
    /* shared memory detach */
    shmdt (barrier_count);

    shmctl (shmid, IPC_RMID, 0);
  
    /* cleanup semaphores */
    cleanup_sem(fork_sem,fork_str);
    cleanup_sem(mutex,lock_str);
    cleanup_sem(sem_master_ready,master_str);
    cleanup_sem(sem_master_completed,completed_str);
    /* unlink prevents the semaphore existing forever */
    /* if a crash occurs during the execution         */
  }
  return(0);
}
