#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "shmWrapper.h"

shmData * shmList[MAX_SHM_SEGMENTS];
key_t globalShmkey;
int globalShmid;

//https://stackoverflow.com/questions/2143404/delete-all-system-v-shared-memory-and-semaphores-on-unix-like-systems
//use whenever you have shm leftover

void initShmWrapper(char * exe_name) {
  int i;
  unsigned char * shmPtr;
  globalShmkey = ftok (exe_name, 1);
  printf("ShmWrapper (shmList %d) (shmData %d) (*shmList %d)\n",(int) sizeof(shmList),sizeof(shmData),sizeof(*shmList));
  globalShmid = shmget (globalShmkey, sizeof (shmData)*MAX_SHM_SEGMENTS, 0644 | IPC_CREAT);
  shmPtr =  (unsigned char *)shmat (globalShmid, NULL, 0);
  printf("shmPtr:0x%08x, size alloc'd %d\n",shmPtr,sizeof (shmData)*MAX_SHM_SEGMENTS);
  for (i=0; i < MAX_SHM_SEGMENTS; i++) {
    shmList[i]=(shmData *) &(shmPtr[sizeof(shmData)*i]);
        printf("shmList[%d]:%08x\n",i,shmList[i]);
    shmList[i]->shmkey = 3*i;
    printf("shmList[%d]->shmkey:%08x\n",i,&(shmList[i]->shmkey));
    shmList[i]->shmid = 3*i+1; 
    printf("shmList[%d]->shmid:%08x\n",i,&(shmList[i]->shmid));  
    shmList[i]->numUsing = 3*i;    
    printf("shmList[%d]->numUsing:%08x\n",i,&(shmList[i]->numUsing));
        printf("value: %d\n\n",shmList[i]->shmkey);
  }
  printf("finished initShmWrapper\n");
}


void createShm(size_t size,int ID) {
  key_t shmkey;
  int shmid;
  int keyID;
  FILE * fp;
  char nameBuff[20];
  /*
  if ((ID-1)/255 > 0) {
    //handle later
    return;
  }
  */
  //printf("createShm Entered\n");
  sprintf(nameBuff,"shm_handle");//,(ID-1)/255);
  fp = fopen(nameBuff,"w");
  if (fp == NULL) {
    printf("File does not exist.\n");
   return;
  }
  printf("-------\n");
  keyID = (ID-1)%255+1;
  shmkey = ftok(nameBuff,keyID);
  shmid = shmget (shmkey, size, 0644 | IPC_CREAT);
  
  //printf("shmList:%08x\n",shmList);
  //printf("shmList[%d]:%08x\n",ID-1,shmList[ID-1]);
  shmList[ID-1]->shmid = shmid;
  shmList[ID-1]->shmkey = shmkey;
  (shmList[ID-1]->numUsing)++;
  printf("Created shared memory segment of size %d with ID %d.\n",size,ID);
}
void * attachShm(int ID) {
  //printf("getPtrShm shmList:%08x\n",shmList[ID]);
  //printf("getPtrShm shmList->shmkey:%08x\n",&(shmList[ID]->shmkey));
  //printf("getPtrShm shmList->shmkey:%d\n",shmList[ID]->shmkey);
  unsigned char * retval;
  //printf("---1----\n");
  if (shmList[ID-1]->numUsing < 1) {
    retval = (void *) NULL;
  }
  //printf("---2----\n");
  retval = (void *) shmat (shmList[ID-1]->shmid, NULL, 0);
  //printf("---3----\n");
  printf("getPtrShm retval:%08x\n",retval);
  return (void *) malloc(sizeof(retval));
}
void detachShm(int ID, void * ptr){
  free(ptr);
}
void deleteShm(int ID){
  
}
