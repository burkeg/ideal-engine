#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "idealEngine.h"
#include "idealEngineTypes.h"

/*
  void map (kv_pairs * key_values) {
  int sum,i,lim;
  sum=0;
  // lim=10000000+rand()%10000000;
  lim=500;
  for (i=0; i < lim; i++) {
  sum += sin((double)i);
  i+=rand()%5;
  }
  }//*/

void map (kv_pairs * key_values) {
  int i;
  char buff[100];
  sprintf(buff,"MAP PRODUCED: ");
  for (i = 0; i < key_values->count; i++) {
    if (strlen(buff) + strlen(key_values->pairs[i]->value) > 99) {
      sprintf(buff+strlen(buff),"...");
      break;
    }
    sprintf(buff+strlen(buff),"[%s] ",key_values->pairs[i]->value);
  }
  sprintf(buff+strlen(buff),"\n");
  printf("%s\n",buff);
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
  bounds = malloc(sizeof(long int *) * NUM_PARTITIONS_MAP);
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    bounds[i]=malloc(sizeof(long int) * 2);
    bounds[i][0]=i*2;
    bounds[i][1]=i*2+1;
  }
  printf("exiting find_partition_bounds\n");
  return bounds;
}

long int ** inputReader(char *inputFilename) {
  FILE * fp;
  long int size;
  int start_indexes[NUM_PARTITIONS_MAP];
  int count,result,base_to_search,len_to_search,flag,sum,i;
  char newlineBuff[100];
  long int ** bounds;
  if ((fp=fopen(inputFilename,"r"))==NULL) {
    printf("failed to read input file\n");
    return (void *) NULL;
  }
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  printf("size: %d\n",(int)size);
  fseek(fp,0,SEEK_SET);
  start_indexes[0]=0;
  count = 1;
  flag=0;
  result=0;
  bounds=find_partition_bounds();
  bounds[0][0]=0;
  for (i=1;i<NUM_PARTITIONS_MAP;i++) {
    //printf("i: %d\n",i);
    base_to_search=(size/NUM_PARTITIONS_MAP)*i;
    len_to_search=5;
    sum=0;
    while(1) {
      
      if (len_to_search+base_to_search >= size-1) {
	len_to_search=size-base_to_search-1;
	flag=1;
      }
      fseek(fp,base_to_search,SEEK_SET);
      printf("base / len_to_search / sum: %d %d %d\n,",(int)base_to_search,(int)len_to_search,sum);
      fread (newlineBuff,sizeof(char),len_to_search,fp);
      printf("newlineBuff: [%s]\n",newlineBuff);
      if ((result=findFirstNewline(newlineBuff,len_to_search)) == -1) {
	if (flag) {
	  printf("failed to partition input data.\n");
	  return (void *) NULL;
	}
	base_to_search+=5;
	continue;
      }
      break;
    }
    start_indexes[i]=base_to_search+result + 1;
    fseek(fp,start_indexes[i],SEEK_SET);
    fread(newlineBuff,1,5,fp);
    bounds[i-1][1]=start_indexes[i]-1;
    bounds[i][0]=start_indexes[i];
    printf("found index %d, [%s].\n",start_indexes[i],newlineBuff);
  }
  bounds[NUM_PARTITIONS_MAP-1][1]=size-1;
  
  for (i = 0; i < NUM_PARTITIONS_MAP; i++) {
    fseek(fp,bounds[i][0],SEEK_SET);
    if (bounds[i][1]-bounds[i][0]+1 >= 90) {
      fread(newlineBuff,sizeof(char),90,fp);
      newlineBuff[90]='\0';
      printf("Partition %d [%d,%d]: {\n%s...}\n",i,bounds[i][0],bounds[i][1],newlineBuff);
    } else {
      fread(newlineBuff,sizeof(char),bounds[i][1]-bounds[i][0]+1,fp);
      newlineBuff[bounds[i][1]-bounds[i][0]+1]='\0';
      printf("Partition %d [%d,%d]: {\n%s}\n",i,bounds[i][0],bounds[i][1],newlineBuff);
    }
  }
  //printf("Exiting inputReader\n");
  return bounds;
  
}  
int findFirstNewline(char * stringData, int len) {
  int i;
  //  printf("findFirstNewline {%s}\n",stringData);
  for (i = 0; i < len; i++) {
    //printf("looking at [%c]\n",stringData[i]);
    if (stringData[i] == '\n') {
      printf("Found first newline, [%s], index %d\n",stringData,i);
      return i;
    }
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

kv_pairs * produce_map_kvs(int mapID,partition_bounds *bounds) {
  //printf("entering: produce_map_kvs\n");
  FILE *fp;
  int size,sum,num_kvs,kvs_index,i;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  kv_pairs * pairs;
  kv_pair ** pairs_new;
  kvs_index=0;
  num_kvs = 10;
  pairs = malloc(sizeof(kv_pairs));
  pairs->pairs = malloc(sizeof(kv_pair*)*num_kvs);
  for (i=0; i < num_kvs; i++) {
    pairs->pairs[i] = malloc(sizeof(kv_pair));
  }
  sum=0;
  size=bounds->end - bounds->start;
  fp = fopen(input_filename,"r");
  fseek(fp,bounds->start,SEEK_CUR);
  while ((read = getline(&line, &len, fp)) != -1) {
    //printf("Retrieved line of length %zu :\n", read);
    //printf("%s", line);
    sum+=read;
    if (read >=MAX_VALUE_LENGTH) {
      printf("Line too long to fix into kv pair. (limit %d, offender %d)\n",MAX_VALUE_LENGTH, (int)read);
      return (void *)NULL;
    }
    //printf("kvs_index: %d\n",kvs_index);
    // if we allocated too many keys, double key array capacity
    if (kvs_index == num_kvs) {
      //printf("@@@@@@@@@DOUBLING MEMORY@@@@@@@@@@@%d\n",mapID);
      num_kvs*=2;
      pairs_new=realloc(pairs->pairs,sizeof(kv_pair*)*num_kvs);
      //check to see if realloc worked
      if (pairs_new == NULL) {
	printf("failed to realloc more kv_pairs\n");
	free(pairs->pairs);
	return (void *)NULL; 
      } else {
	//since realloc worked, allocate each kv_pair
	//printf("successfully realloced, now at %d capacity\n",num_kvs);
	pairs->pairs = pairs_new;
	for (i = num_kvs/2; i < num_kvs; i++) {
	  pairs->pairs[i]=malloc(sizeof(kv_pair));
	  if (pairs->pairs[i] == NULL) {
	    printf("failed to alloc a kv_pair\n");
	    return (void *)NULL;
	  }
	}
      }
    }
    
    //guarantees that pairs->pairs[kvs_index] is allocated
    //line also guaranteed to fit into value string buffer
    //printf("line:[%s] size: %d\n",line,read);
    strncpy(&(pairs->pairs[kvs_index]->value[0]),line,read);
    pairs->pairs[kvs_index]->value[read-1]='\0';
    kvs_index++;
    if (sum > size) {
      break;
    }
  }
  
  free(line);
  pairs->count = kvs_index;
  /*
    for (i = 0; i < pairs->count; i++) {
    printf("value %d: [%s]\n",i,pairs->pairs[i]->value);
    }
  //*/
  return pairs;
}
