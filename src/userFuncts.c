#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "idealEngine.h"

void map () {
  int sum,i,lim;
  sum=0;
  // lim=10000000+rand()%10000000;
  lim=500;
  for (i=0; i < lim; i++) {
    sum += sin((double)i);
    i+=rand()%5;
  }
}

void reduce () {
  int sum,i,lim;
  sum=0;
  //lim=10000000+rand()%10000000;
  lim=500;
  for (i=0; i < lim; i++) {
    sum += sin((double)i);
    i+=rand()%5;
  }
}

/*
  Must return NUM_PARTITIONS_MAP long int *
  where each long int * has 2 elements, 
  start (inclusive), end (exclusive)
  
  start and end are the byte locations that
  partition the input file into each
  NUM_PARTITIONS_MAP segments.

  http://www.cplusplus.com/reference/cstdio/ftell/
*/
long int ** find_partition_bounds() {
  long int ** bounds;
  int i;
  bounds = malloc(sizeof(long int) * NUM_PARTITIONS_MAP);
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    bounds[i]=malloc(sizeof(long int) * 2);
    bounds[i][0]=i*2;
    bounds[i][1]=i*2+1;
  }
  return bounds;
}

void inputReader(char *inputFilename) {
  FILE * fp;
  long int size;
  int start_indexes[NUM_PARTITIONS_MAP];
  int count,result,base_to_search,len_to_search,flag,sum,i;
  char newlineBuff[20];
  long int ** bounds;
  if ((fp=fopen(inputFilename,"r"))==NULL) {
    printf("failed to read input file\n");
    return;
  }
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  printf("size: %d\n",size);
  fseek(fp,0,SEEK_SET);
  start_indexes[0]=0;
  count = 1;
  flag=0;
  result=0;
  bounds=find_partition_bounds();
  bounds[0][0]=0;
  for (i=1;i<NUM_PARTITIONS_MAP;i++) {
    base_to_search=(size/NUM_PARTITIONS_MAP)*i;
    len_to_search=5;
    sum=0;
    while(1) {
      if (len_to_search+base_to_search >= size-1) {
	len_to_search=size-base_to_search-1;
	flag=1;
      }
      fseek(fp,base_to_search,SEEK_SET);
      fread (newlineBuff,sizeof(char),len_to_search,fp);
      if ((result=findFirstNewline(newlineBuff,len_to_search)) == -1) {
	if (flag) {
	  printf("failed to partition input data.\n");
	  return;
	}
	sum+=result;
	base_to_search+=5;
	continue;
      }
      sum+=result;
      break;
    }
    start_indexes[i]=(size/NUM_PARTITIONS_MAP)*i+sum+1;
    //fseek(fp,start_indexes[i],SEEK_SET);
    //fread(newlineBuff,1,5,fp);
    bounds[i-1][1]=start_indexes[i]-1;
    bounds[i][0]=start_indexes[i];
      //printf("found index %d, [%s].\n",start_indexes[i],newlineBuff);
  }
  bounds[NUM_PARTITIONS_MAP-1][1]=size-1;
  
  for (i = 0; i < NUM_PARTITIONS_MAP; i++) {
    fseek(fp,bounds[i][0],SEEK_SET);
    fread(newlineBuff,sizeof(char),bounds[i][1]-bounds[i][0]+1,fp);
    newlineBuff[bounds[i][1]-bounds[i][0]+1]='\0';
    printf("Partition %d: {\n%s}\n",i,newlineBuff);
  }
  
}  
int findFirstNewline(char * stringData, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (stringData[i] == '\n') 
      return i;
  }
  return -1;
}
/*
  Must return NUM_PARTITIONS_MAP strings
  each string can be up to 255 characters long
*/
char ** specify_intermediate_filenames() {
  char ** names;
  int i;
  names = malloc(sizeof(char *) * NUM_PARTITIONS_MAP);
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    names[i]=malloc(sizeof(char)*255);
    sprintf(names[i],"intermediate_%03d",i);
  }
  return names;
}

kv_pairs * produce_map_kvs(int mapID,partition_bounds bounds) {
  return (void *)NULL;
}
