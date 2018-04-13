#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "shmWrapper.h"

//htop and pkill are useful commands to debug

shmData * shmList[MAX_SHM_SEGMENTS];
key_t globalShmkey;
int globalShmid;
unsigned char * shmPtr;

//https://stackoverflow.com/questions/2143404/delete-all-system-v-shared-memory-and-semaphores-on-unix-like-systems
//use whenever you have shm leftover

void initShmWrapper(char * exe_name) {
  int i;
  globalShmkey = ftok (exe_name, 1);
  //printf("ShmWrapper (shmList %d) (shmData %d) (*shmList %d)\n",(int) sizeof(shmList),sizeof(shmData),sizeof(*shmList));
  globalShmid = shmget (globalShmkey, sizeof (shmData)*MAX_SHM_SEGMENTS, 0644 | IPC_CREAT);
  shmPtr =  (unsigned char *)shmat (globalShmid, NULL, 0);
  //printf("shmPtr:0x%08x, size alloc'd %d\n",shmPtr,sizeof (shmData)*MAX_SHM_SEGMENTS);
  for (i=0; i < MAX_SHM_SEGMENTS; i++) {
    shmList[i]=(shmData *) &(shmPtr[sizeof(shmData)*i]);
    //printf("shmList[%d]:%08x\n",i,shmList[i]);
    shmList[i]->shmkey = 5*i;
    //printf("shmList[%d]->shmkey:%08x\n",i,&(shmList[i]->shmkey));
    shmList[i]->shmid = 5*i+1; 
    //printf("shmList[%d]->shmid:%08x\n",i,&(shmList[i]->shmid));  
    shmList[i]->inuse = 0;
    //printf("shmList[%d]->inuse:%08x\n\n",i,&(shmList[i]->inuse));
  }
  //printf("finished initShmWrapper\n");
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
  sprintf(nameBuff,"shm_handle%d",(ID-1)/255);
  fp = fopen(nameBuff,"w");
  if (fp == NULL) {
    printf("File does not exist.\n");
   return;
  }
  //printf("-------\n");
  keyID = (ID-1)%255+1;
  shmkey = ftok(nameBuff,keyID);
  shmid = shmget (shmkey, size, 0644 | IPC_CREAT);
  
  //printf("shmList:%08x\n",shmList);
  //printf("shmList[%d]:%08x\n",ID-1,shmList[ID-1]);
  shmList[ID-1]->shmid = shmid;
  shmList[ID-1]->shmkey = shmkey;
  shmList[ID-1]->inuse = 1;
  //printf("Created shared memory segment of (size %d) (ID %d) &(shmid %08x) &(shmkey %08x) (shmid %08x) (shmkey %08x)\n",size,ID,&(shmList[ID-1]->shmid),&(shmList[ID-1]->shmkey),shmList[ID-1]->shmid,shmList[ID-1]->shmkey);
}

//returns shmid
int attachShm(int ID) {
  //printf("attachShm shmList:%08x\n",shmList[ID]);
  //printf("attachShm shmList->shmkey:%08x\n",&(shmList[ID]->shmkey));
  //printf("attachShm shmList->shmkey:%d\n",shmList[ID]->shmkey);
  if (shmList[ID-1]->inuse == 0) {
    return -1;
  }
  return shmList[ID-1]->shmid;
}
void detachShm(int ID, void * ptr){
  if (shmList[ID-1]->inuse == 0 || shmdt(ptr) == -1) {
    printf("Failed to detach from memory\n");
    return;
  }
  
}
void deleteShm(int ID){
  if (shmList[ID-1]->inuse == 0) {
    //printf("Shm not currently in use!\n");
    return;
  }
  if (shmctl(shmList[ID-1]->shmid, IPC_RMID, NULL) == -1) {
    printf("Failed to remove shm segment\n");
    return;
  }
  shmList[ID-1]->inuse = 0;
}

void closeShmWrapper() {
  if (shmdt(shmPtr) == -1) {
    printf("Failed to detach from memory\n");
    return;
  }
  if (shmctl(globalShmid, IPC_RMID, NULL) == -1) {
    printf("Failed to remove shm segment\n");
    return;
  }
}
