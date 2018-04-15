#include "idealEngine.h"
#include <string.h>
//#include <bsd/string.h>
//#include "dataManager.c"

kv_pair **key_values;
int idx;
//kv_pair key_values[MAX_KEYS_PER_MAPPER];

void initDataManager() {
  key_values = malloc(sizeof(kv_pair)*MAX_KEYS_PER_MAPPER);
  idx=0;
}

void emit(kv_pair * kv) {
  key_values[idx]->key=kv->key;
  strcpy(kv->value,key_values[idx]->value);
  idx++;
}

