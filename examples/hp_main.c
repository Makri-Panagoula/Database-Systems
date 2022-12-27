#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "data3.db"

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

  /* Create Heap File */
  if(HP_CreateFile(FILE_NAME) != 0)
    printf("Error! Could not create file\n");

  /* Open Heap File */
  HP_info* info = HP_OpenFile(FILE_NAME);
  if(info == NULL)
      printf("Error! Could not open file\n");

  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    int code = HP_InsertEntry(info, record);;
    if(code == -1) {
      printf("Insert Entry: Not okay\n");
    }
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("Searching for entry with id: %d\n",id);
  HP_GetAllEntries(info, id);

  if(HP_CloseFile(info) != 0)
    printf("Could not close Heap file");
    
  BF_Close();
}
