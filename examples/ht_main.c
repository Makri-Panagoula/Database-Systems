#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"

#define RECORDS_NUM 150 // you can change it if you want
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

  /* Create Hash File */
  if(HT_CreateFile(FILE_NAME, 10) != 0)
    printf("Error! Could not create file\n");

  /* Open Hash File */
  HT_info* info = HT_OpenFile(FILE_NAME);
  if(info == NULL)
      printf("Error! Could not open file\n");

  Record record;
  srand(12569874);
  printf("---------- Insert Entries ----------\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HT_InsertEntry(info, record);
    // printRecord(record);
  }

  printf("---------- PrintAllEntries ----------\n");
  int id = rand() % RECORDS_NUM;
  printf("Id is : %d\n", id);

  if(HT_GetAllEntries(info, &id) == 0)
     printf("Error in HT_GetAllEntries!\n");

  printf("\n---------- STATISTICS ----------\n");
  if(HashStatistics(FILE_NAME) != 0)
    printf("Error in HashStatistics\n");

  HT_CloseFile(info);
  BF_Close();
}
