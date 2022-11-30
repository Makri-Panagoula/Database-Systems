#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName, Record_Attribute attr){

  CALL_BF(BF_CreateFile(filename));
  int desc;
  CALL_BF(BF_OpenFile(filename,&desc)); //Creating and opening the heap file in order to place the metadata we need

  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(desc, block)); //Allocating the first block for the heap file

  char* before= BF_Block_GetData(block); //Getting its data
  memcpy(before, &attr, sizeof(Record_Attribute)); //Using memcpy to change the block's data 

  //Make it pinned so that we won't be removed.

  BF_Block_SetDirty(block); 
 //pinned????

  BF_Block_Destroy(block); //Taking care of the bytes allocated from BF_Block_Init()
  CALL_BF(BF_CloseFile(desc));

  return 0;
}

HP_info* HP_OpenFile(char *fileName){
    return malloc(sizeof(HP_info));

}


int HP_CloseFile( HP_info* header_info ){
    return -1;
}

int HP_InsertEntry(HP_info* header_info, Record record){
        return -1;
}

int HP_GetAllEntries(HP_info* header_info, void *value ){
    return -1;
}

