#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){

  BF_Block* block;
  BF_Block_Init(&block);

  //Create the heap file
  CALL_BF(BF_CreateFile(fileName));

  // Create a block in the file and save in it some arbitrary metadata.
  
  int desc;
  CALL_BF(BF_OpenFile(fileName,&desc)); 

  CALL_BF(BF_AllocateBlock(desc, block)); 
  char* before= BF_Block_GetData(block);
  memcpy(before, &attr, sizeof(Record_Attribute));  

  BF_Block_SetDirty(block); 
  CALL(BF_UnpinBlock(block));

  BF_Block_Destroy(block); 
  CALL_BF(BF_CloseFile(desc));

  return 0;
}

//Μπορει να κραταει και την πληροφορια οτι το αρχειο ειναι οντως αρχειο σωρου

HP_info* HP_OpenFile(char *fileName){

  BF_Block* metadata;
  BF_Block_Init(&metadata);

 //Get metadata from block 0 to initialize the struct

  int desc;
  CALL_BF(BF_OpenFile(fileName,&desc)); 
  CALL_BF(BF_GetBlock(desc,0 ,&metadata)); 
  Record_Attribute* key = (Record_Attribute*) BF_Block_GetData(metadata);

  //If the key holds an unexpected value, we don't have a heap file 

  if(key == NULL || (*key != ID && *key != NAME && *key != SURNAME && *key != CITY))
    return  NULL;

  HP_info* info = malloc(sizeof(HP_info));
  info->fileDesc = desc;
  info->keyAttribute = *key;
  
  BF_Block_Destroy(&metadata);
  CALL_BF(BF_CloseFile(desc));
  
  return info;
}


int HP_CloseFile( HP_info* header_info ){
    return -1;
}

//Σε καθε block πρέπει να κραταμε εναν δεικτη που θα μας πηγαίνει στο επόμενο block
//Στα μεταδεδομενα κραταμε που ακριβως θα γινει η εισαγωγη

int HP_InsertEntry(HP_info* header_info, Record record){

  int bytes_left = BF_BLOCK_SIZE - header_info->bytes;
  int to_add = sizeof(record);

  if(to_add > bytes_left) {

    BF_Block* new;
    BF_AllocateBlock(header_info->fileDesc,new);

  }
        return -1;
}

int HP_GetAllEntries(HP_info* header_info, void *value ){
    return -1;
}

