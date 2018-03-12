#include "idealEngine.h"

int main() {
  int i;
  int *data;
  FILE *fp;
  int * buffer;
  int numEl;
  long size;
  partition_bounds partition_starts[NUM_PARTITIONS];
  fp = fopen("data.hex","r");
  if (fp==NULL) {
    printf("Cannot open file\n");
    return 0;
  }
  
  fseek (fp, 0, SEEK_END);
  size=ftell(fp);
  fseek( fp, 0, SEEK_SET );
  fread(&numEl,4,1,fp);
  printf("%d\n",numEl);
  
  buffer = (int *)malloc(sizeof(int)*size);
  if (buffer==NULL) {
    printf("Cannot allocate buffer\n");
    return 0;
  }
  fread(buffer,4,numEl,fp);
    
  for (i = 0; i < numEl; i++) {
    printf("%d\n",(int)buffer[i]);
  }
  printf("%d\n",NUM_PARTITIONS);
  //mapping = (int *) mmap(startingArr,400,PROT_WRITE,);

  fseek(fp,4,SEEK_SET);
  partition_starts[0].start=ftell(fp);
  for (i=1; i < NUM_PARTITIONS; i++) {
    fseek(fp,size/NUM_PARTITIONS,SEEK_CUR);
    partition_starts[i-1].end=ftell(fp)-1;
    partition_starts[i].start=ftell(fp);
  }
  fseek(fp,0,SEEK_END);
  partition_starts[NUM_PARTITIONS-1].end=ftell(fp);
  
  for (i=0; i < NUM_PARTITIONS; i++) {
    printf("%d:(%ld,%ld)\n",i,partition_starts[i].start,partition_starts[i].end);
  }
  fclose(fp);
  return(0);
}
