#include "idealEngine.h"
#include <string.h>

simpleArray * allocBuff(int capacity) {
  simpleArray *sArr = malloc(sizeof(simpleArray));
  sArr->data=malloc(sizeof(int)*capacity);
  sArr->size=capacity;
  sArr->head=0;
  return sArr;
}

int len(simpleArray * buffer) {
  return buffer->head;
}
int peek(simpleArray * buffer) {
  if (buffer->head <= 0) {
    return -1; //silently continue;
  }
  return buffer->data[buffer->head-1];
}

int pop(simpleArray * buffer) {
  if (buffer->head <= 0) {
    return -1; //silently continue;
  }
  (buffer->head)--;
  int retval=buffer->data[buffer->head];
  return retval;
}

void push(simpleArray * buffer,int value) {
  if (buffer->head == buffer->size) {
    return; //silently continue;
  }
  buffer->data[buffer->head]=value;
  (buffer->head)++;
//  printf("[0]=%d\n",buffer->data[0]);
}

int isFull(simpleArray * buffer) {
  return buffer->head==buffer->size;
}

int isEmpty(simpleArray * buffer) {
  return !(buffer->head);
}

void deallocBuff(simpleArray * buffer) {
  free(buffer->data);
  free(buffer);
}

void removeByValue(simpleArray * buffer, int value) {
  int i;
  for (i = 0; i < buffer->head; i++) {
    if (buffer->data[i] == value)
      break;
  }
  if (i==buffer->head)
    return;
  for (i = i; i<buffer->head; i++) {
    buffer->data[i]=buffer->data[i+1];
  }
  (buffer->head)--;  
}

void printBuff(simpleArray * buffer) {
  int i;
  char strBuff[256];
  sprintf(strBuff,"{");
  for (i = 0; i < buffer->head;i++) {
    sprintf(strBuff+strlen(strBuff),"%d,",buffer->data[i]);
  }
  sprintf(strBuff+strlen(strBuff),"}\n");
  printf("%s",strBuff);
}

void demo() {
  simpleArray *arr=allocBuff(4);
  int i;
  printBuff(arr);
  printf("Peek into empty:%d\n",peek(arr));
  printf("Pop empty array:%d\n",pop(arr));
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  for (i=4;i<15;i+=3) {
    push(arr,i);
    printf("push %d\n",i);
    printBuff(arr);
  }

  printf("isFull:%d, len:%d\n",isFull(arr),len(arr));
  removeByValue(arr,13);  
  printBuff(arr);
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  removeByValue(arr,4);  
  printBuff(arr);
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  removeByValue(arr,7);  
  printBuff(arr);
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  removeByValue(arr,10);  
  printBuff(arr);
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  removeByValue(arr,13);  
  printBuff(arr);
  printf("isEmpty:%d, len:%d\n",isEmpty(arr),len(arr));
  
  deallocBuff(arr);
}
