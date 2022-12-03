#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#define MAX_OPEN_FILES 20
#define HP_ERROR -1;

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
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
  BF_OpenFile(fileName, &desc); 

  CALL_BF(BF_AllocateBlock(desc, block)); 
  char* before= BF_Block_GetData(block);
  memcpy(before, "Heap", strlen("Heap")+1);  

  BF_Block_SetDirty(block); 
  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block); 
  CALL_BF(BF_CloseFile(desc));
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  // Open file 
  int desc;
  BF_OpenFile(fileName, &desc); 

  BF_Block* metadata;
  BF_Block_Init(&metadata);

  // Get data from first block
  BF_GetBlock(desc, 0, metadata); 
  char* heap = (char*)BF_Block_GetData(metadata);

  // If the key holds an unexpected value, then this is not a heap file 
  if(strcmp(heap,"Heap") != 0)
    return NULL;  

  //Allocate memory address for the pointer we will return and initialize its fields.
  HP_info* info = malloc(sizeof(HP_info));
  info->fileDesc = desc;

  //The first block holds only the metadata
  info->records = 0;
  //The bytes of records per block will be its total size minus the pointer part (its metadata)
  info->tot_records = (BF_BLOCK_SIZE - sizeof(BF_Block*)) / sizeof(Record);

  BF_Block_Destroy(&metadata);
  return info;
}


int HP_CloseFile(HP_info* header_info ){
  // Close file with the identical fileDesc
  CALL_BF(BF_CloseFile(header_info->fileDesc));
  return 0;
}


int HP_InsertEntry(HP_info* header_info, Record record){

  int blocks;
  BF_Block* prev;
  BF_Block_Init(&prev);
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks));

  //Getting data from the last block 
 //Block enumeration starts from 0

  CALL_BF(BF_GetBlock(header_info->fileDesc , blocks -1  , prev));   
  Record* prev_data = (Record*) BF_Block_GetData(prev);


  if( header_info->records == header_info->tot_records  || blocks == 1) {

    BF_Block* new;
    BF_Block_Init(&new);
    CALL_BF(BF_AllocateBlock(header_info->fileDesc,new));

    //Store pointer to new block as metadata
    void* addr = prev_data + header_info->tot_records;
    memcpy(addr , new , sizeof(BF_Block*));

    //Put record in the new block
    Record* new_block= (Record*) BF_Block_GetData(new);
    new_block[0]=record;
    header_info->records = 1;  

    //Take care of the block
    BF_Block_SetDirty(new); 
    CALL_BF(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);
  }

  //We store the record in the last block
  else {

    prev_data[header_info->records]=record;
    header_info->records++;
  }

  BF_Block_SetDirty(prev); 
  CALL_BF(BF_UnpinBlock(prev));  

  BF_Block_Destroy(&prev);
  return 0;
}

int HP_GetAllEntries(HP_info* header_info, int id ){

  BF_Block* cur;
  BF_Block_Init(&cur);
  printf("okay");
  int blocks;
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks));
  printf("\ncounting blocks %d\n", blocks);


  int records = header_info->tot_records;

  for(int i=0; i < blocks; i++) {
    printf("\n%d",i);

    //Getting data from each block 
    CALL_BF(BF_GetBlock(header_info->fileDesc , i , cur));   

    if(i == blocks - 1) {
      records = header_info->records;
    }
  
    Record* data = (Record*) BF_Block_GetData(cur);   

    for(int j=0; j < records ; j++) {        //For each record inside the block

      if(data[j].id == id) {
        printRecord(data[j]);
      }
    } 

    CALL_BF(BF_UnpinBlock(cur));  

  }

  BF_Block_Destroy(&cur);
  return blocks;
}

