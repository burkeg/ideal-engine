#include "idealEngine.h"

/*
https://stackoverflow.com/questions/16400820/c-how-to-use-posix-semaphores-on-forked-processes
*/
int main() {
  int i;
  int *data;
  FILE *fp;
  int * buffer;
  int numEl;
  long size;    
  key_t shmkey;
  int shmid;   
  sem_t *mutex;
  pid_t pid;
  partition_bounds partition_starts[NUM_PARTITIONS_MAP];

  
  /* initialize a shared variable in shared memory */
  shmkey = ftok ("/dev/null", 5);       /* valid directory name and a number */
  printf ("shmkey for p = %d\n", shmkey);
  shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
  if (shmid < 0){                           /* shared memory error check */
    perror ("shmget\n");
    exit (1);
  }

  //  p = (int *) shmat (shmid, NULL, 0);   /* attach p to shared memory */
  // *p = 0;
  //  printf ("p=%d is allocated in shared memory.\n\n", *p);
  
  
  /* initialize semaphores for shared processes */
  mutex = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 0); 

  pid = fork();
  printf("pid: %d",pid);
  if (pid==0) {
    //Master
    
    
    fp = fopen("data.hex","r");
    if (fp==NULL) {
      printf("Cannot open file\n");
      return -1;
    }
  
    fseek (fp, 0, SEEK_END);
    size=ftell(fp);
    fseek( fp, 0, SEEK_SET );
    fread(&numEl,4,1,fp);
    printf("%d\n",numEl);
  
    buffer = (int *)malloc(sizeof(int)*size);
    if (buffer==NULL) {
      printf("Cannot allocate buffer\n");
      return -1;
    }
    fread(buffer,4,numEl,fp);
    
    for (i = 0; i < numEl; i++) {
      printf("%d\n",(int)buffer[i]);
    }
    printf("%d\n",NUM_PARTITIONS_MAP);
    //mapping = (int *) mmap(startingArr,400,PROT_WRITE,);

    fseek(fp,4,SEEK_SET);
    partition_starts[0].start=ftell(fp);
    for (i=1; i < NUM_PARTITIONS_MAP; i++) {
      fseek(fp,size/NUM_PARTITIONS_MAP,SEEK_CUR);
      partition_starts[i-1].end=ftell(fp)-1;
      partition_starts[i].start=ftell(fp);
    }
    fseek(fp,0,SEEK_END);
    partition_starts[NUM_PARTITIONS_MAP-1].end=ftell(fp);
  
    for (i=0; i < NUM_PARTITIONS_MAP; i++) {
      printf("%d:(%ld,%ld)\n",i,partition_starts[i].start,partition_starts[i].end);
    }
    fclose(fp);
    printf("\nJust Exiting...\n");
    //    sem_post(mutex);
    return (0);
  }
  sem_wait(mutex);
  printf("\nEntered..\n");
  printf("\n--------------\n");

  //creates n workers
  for (i = 0; i < NUM_PARTITIONS_MAP + NUM_PARTITIONS_REDUCE - 1; i++) {
    pid = fork ();
    if (pid < 0) {
      /* check for error      */
      sem_unlink ("pSem");   
      sem_close(mutex);  
      /* unlink prevents the semaphore existing forever */
      /* if a crash occurs during the execution         */
      printf ("Fork error.\n");
    }
    else if (pid == 0)
      break;                  /* child processes */
  }
  
  printf("My pid is %d.\n",pid);


  
  /* shared memory detach */
  //  shmdt (p);
  shmctl (shmid, IPC_RMID, 0);

  /* cleanup semaphores */
  sem_unlink ("pSem");   
  sem_close(mutex);  
  /* unlink prevents the semaphore existing forever */
  /* if a crash occurs during the execution         */
  return(0);
}
