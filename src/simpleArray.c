#include "idealEngine.h"


simpleArray * allocBuff(int size) {
  simpleArray *sArr = malloc(sizeof(simpleArray));
  sArr->data=malloc(sizeof(int)*size);
  sArr->size=size;
  sArr->head=0;
  return sArr;
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
  printf("-------\n");
  for (int i = 0; i < buffer->head;i++) {
    printf("%d,",buffer->data[i]);
  }
  printf("\n");
}

void demo() {
  simpleArray *arr=allocBuff(4);
  int i;
  printBuff(arr);
  printf("Peek into empty:%d\n",peek(arr));
  printf("Pop empty array:%d\n",pop(arr));
  printf("isEmpty:%d\n",isEmpty(arr));
  for (i=4;i<15;i+=3) {
    push(arr,i);
    printf("push %d\n",i);
    printBuff(arr);
  }

  removeByValue(arr,13);  
  printBuff(arr);
  printf("isEmpty:%d\n",isEmpty(arr));
  removeByValue(arr,4);  
  printBuff(arr);
  printf("isEmpty:%d\n",isEmpty(arr));
  removeByValue(arr,7);  
  printBuff(arr);
  printf("isEmpty:%d\n",isEmpty(arr));
  removeByValue(arr,10);  
  printBuff(arr);
  printf("isEmpty:%d\n",isEmpty(arr));
  removeByValue(arr,13);  
  printBuff(arr);
  printf("isEmpty:%d\n",isEmpty(arr));
  
  deallocBuff(arr);
}
