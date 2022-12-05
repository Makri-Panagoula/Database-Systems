#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"

#define RECORDS_NUM 200 // you can change it if you want
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);

  HT_CreateFile(FILE_NAME,10);
  HT_info* info = HT_OpenFile(FILE_NAME);
  printf("%p",info);

  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    printf("Record id: %d\n", id);
    record = randomRecord();
    HT_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  // int id = rand() % RECORDS_NUM;
  // HT_GetAllEntries(info, &id);

  HT_CloseFile(info);
  BF_Close();
}
