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

  // Open the file to get access to it
  int desc;
  BF_OpenFile(fileName, &desc); 

  // Get the data of the block and write to it 
  // identifying it as a heap file
  CALL_BF(BF_AllocateBlock(desc, block)); 
  char* before = BF_Block_GetData(block);

  // Allocate memory address for the pointer 
  HP_info info ;

  // Initializing HP_INFO fields
  info.fileDesc = desc;
  info.records = 0;    // The first block holds only the metadata

  // The available bytes for records per block will be its total size minus the pointer part (its metadata)
  // The number of records per block will be the available bytes divided by the size of each record (records have a fixed size)
  info.tot_records = (BF_BLOCK_SIZE - sizeof(BF_Block*)) / sizeof(Record);
  info.heap = "Heap";

  //Copy the struct in the memory
  memcpy(before, &info, sizeof(HP_info));  

 // We have changed the content of the block, so we set it dirty to be recopied in hard disk
 // We don't unpin, we want to keep the first block with the metadata in the heap.
  BF_Block_SetDirty(block); 

  BF_Block_Destroy(&block);       // Take care of initialized block
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
  HP_info* info = (HP_info*)BF_Block_GetData(metadata);

  // If the key holds an unexpected value, then this is not a heap file
  if( info == NULL || strcmp(info->heap,"Heap") != 0)
    return NULL;  

  //Initialize the pointer with the struct values.
  HP_info* to_return = malloc(sizeof(HP_info));
  memcpy(to_return , info , sizeof(HP_info));

  // Done with the block.Don't unpin!
  BF_Block_Destroy(&metadata);

  return to_return;
}

// We keep the first block pinned until we close the file
int HP_CloseFile(HP_info* header_info ){

  // Close file with the identical fileDesc
  BF_Block* metadata;
  BF_Block_Init(&metadata);
  BF_GetBlock(header_info->fileDesc, 0, metadata);   

  CALL_BF(BF_UnpinBlock(metadata));
  BF_Block_Destroy(&metadata);

  CALL_BF(BF_CloseFile(header_info->fileDesc));
  free(header_info);
  return 0;
}


int HP_InsertEntry(HP_info* header_info, Record record){

  //Number of blocks currently in buffer
  int blocks;
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks)); 

  // Get data from the last block 
  BF_Block* prev;
  BF_Block_Init(&prev);

  CALL_BF(BF_GetBlock(header_info->fileDesc, blocks - 1, prev));   // Remember!! Block enumeration starts from 0
  Record* prev_data = (Record*) BF_Block_GetData(prev);

  // If we have reached the number of records the block can store or we are in the first one 
  // that only stores the metadata, we allocate a new block, we store its pointer
  // to the metadata part of the previous one (the formerly last) and we place the record in a new block.

  if( header_info->records == header_info->tot_records || blocks == 1) {

    BF_Block* new;
    BF_Block_Init(&new);
    CALL_BF(BF_AllocateBlock(header_info->fileDesc, new));

    // Put record in the new block + fix struct data
    Record* new_block = (Record*)BF_Block_GetData(new);
    new_block[0] = record;
    header_info->records = 1;  

    // Store pointer to new block as metadata
    void* addr;
    // The first block does not contain a pointer to the next block, only metadata of the heap file
    if(blocks != 1){   
      addr = prev_data + header_info->tot_records;   // Find memory address with offset
      memcpy(addr, new, sizeof(BF_Block*));
    }

    // New block changed and now we are done with it 
    BF_Block_SetDirty(new); 
    CALL_BF(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);  

    if(blocks == 1) {
      // We keep the first block pinned until we close the file
      BF_Block_Destroy(&prev);
      return 0;
    }
  }

  // Store the record in the last block
  else {

    prev_data[header_info->records] = record;
    header_info->records++;
  }

  // Last block changed and now we are done with it 
  BF_Block_SetDirty(prev); 
  CALL_BF(BF_UnpinBlock(prev));  
  BF_Block_Destroy(&prev);

  return 0;
}

int HP_GetAllEntries(HP_info* header_info, int id ){

  BF_Block* cur;
  BF_Block_Init(&cur);

  int blocks;
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks));

  // Total number of records in each block
  int records = header_info->tot_records;

  for(int i = 0; i < blocks; i++) {

    // Get data from each block 
    CALL_BF(BF_GetBlock(header_info->fileDesc , i , cur));   

    // If we are in the last block it might not have the max number of records it
    // can store, so we access only the existing ones

    if(i == blocks - 1) {
      records = header_info->records;
    }
  
    Record* data = (Record*)BF_Block_GetData(cur);   

    for(int j = 0; j < records; j++) {        // For each record inside the block
      if(data[j].id == id) {
        printRecord(data[j]);
      }
    } 

    // Done with the block, we don't need it in the buffer memory anymore, it can be replaced unless it's the first one.
    if(i!=0)
      CALL_BF(BF_UnpinBlock(cur));  
  }

  BF_Block_Destroy(&cur);
  
  return blocks;
}

