# DBSM

## Heap
As an identifier in the `HP_info` struct, we have a string that has the value "Heap" so that we know that the structure is indeed a Heap. We do not unpin the 1st block except in the `Closefile` function. Data is inserted at the end of the file, and if it doesn't fit in the already existing block, we create a new one.

**HP_GetAllEntries:** We access all entries sequentially.

## Hash Table
In the block 0 of the file, we store metadata, including an array where the index corresponds to the bucket number. At each position of the array, there is the number of the last block of the corresponding bucket. Additionally,

**HT_CreateFile:** When creating metadata, we also create a number of blocks corresponding to the number of buckets so that mapping to the array can occur. Since these are initial blocks and have no predecessor, the value in the metadata will be -1. We do not unpin the first block of the file as it holds the metadata, and we want it to remain in memory.

**HT_InsertEntry:** When inserting a record, we check if the last block fits; otherwise, we create a new one and initialize its metadata using the `create_metadata` function. We update the metadata of the file, as the last block of the bucket has changed, and write these changes to block 0. If the last block can accommodate more entries, the entry is inserted there. Finally, we increment the number of entries in that particular block.

**HT_CloseFile:** Here, block 0 is unpinned.

**HT_GetAllEntries:** We hash the key given by the function and access the bucket blocks starting from the last and recursively moving backward to the first, checking the records of each block. It returns the number of blocks accessed.

**HashStatistics:** Following a similar logic to the `HT_GetAllEntries` function, we now access the file's metadata first through block 0. We calculate how many blocks a file has and from all the buckets, find which has the minimum, mean, and maximum number of records. Additionally, we calculate the mean number of blocks each bucket has and the number of buckets that have overflow blocks, along with how many blocks they are for each bucket as required.

Observing that entries will be evenly distributed in buckets based on the hash function using the incrementing id number, we expect equal minimum, mean, and maximum numbers of records per block by running the `HashStatistics` function.

## Makefile
Use the Makefile to compile, run, and clean using the following commands:

**Heap**:

```bash
$ make hp
$ make runhp
$ make clean
```
**HashTable**:

```bash
$ make ht
$ make runht
$ make clean
```

## Secondary Hash Table

As an identifier in the `SHT_info` struct, we have a string that has the value "Secondary HashTable" so that we know that the structure is indeed a Secondary Hash Table. We do not unpin the 1st block except in the `Closefile` function. Data is inserted at the end of the file, and if it doesn't fit in the already existing block, we create a new one. We follow the same implementation as the Hash Table, with the difference that we insert pairs consisting of the name of each record and the number of the block it resides in the Hash Table.

**sht_metadata:**
We call this function each time we want to create a new block in each bucket and initialize the variables in an `SHT_block_info`.

**SHT_OpenSecondaryIndex:**
We access the first block with the metadata of the file and copy it to another pointer, which we then return.

**SHT_SecondaryGetAllEntries:** 
We access all entries sequentially.

In the block 0 of the file, we store metadata, including an array where the index corresponds to the bucket number. At each position of the array, there is the number of the last block of the corresponding bucket.
Additionally, in the `HT_info` struct, we store information, a string that when it has the value "HashTable" to know that the structure is indeed a HashTable.
The metadata of each block (`HT_block_info`) holds the number of the previous block and the current records in the block.

**SHT_CreateSecondaryIndex:** 
When creating metadata, we also create a number of blocks corresponding to the number of buckets so that mapping to the array can occur. Since these are initial blocks and have no predecessor, the value in the metadata will be -1. We do not unpin the first block of the file as it holds the metadata, and we want it to remain in memory.

**SHT_SecondaryInsertEntry:**
When inserting a record, we check if the last block fits; otherwise, we create a new one and initialize its metadata using the `sht_metadata` function. We update the metadata of the file, as the last block of the bucket has changed, and write these changes to block 0. If the last block can accommodate more entries, the entry is inserted there. Finally, we increment the number of entries in that particular block.

**SHT_CloseSecondaryIndex:** 
Here, block 0 is unpinned.

**SHT_SecondaryGetAllEntries:** 
We hash the key given by the function and access the bucket blocks starting from the last and recursively moving backward to the first, checking the records of each block. After finding the required name, we have access to the block of the entry in the Hash Table, and then we access all the entries to locate the entire record. It returns the number of blocks accessed.

**HashStatistics:** 
Following a similar logic to the `SHT_SecondaryGetAllEntries` function, we now access the file's metadata first through block 0. We calculate how many blocks a file has and from all the buckets, find which has the minimum, mean, and maximum number of records. Additionally, we calculate the mean number of blocks each bucket has and the number of buckets that have overflow blocks, along with how many blocks they are for each bucket as required.

Observing that entries will be evenly distributed in buckets based on the hash function using the incrementing id number, we expect equal minimum, mean, and maximum numbers of records per block by running the `HashStatistics` function.

Because the structures and their fields differ, we call different functions depending on which structure we are interested in to keep the code clear. However, the logic remains exactly the same.


## Makefile
Use the Makefile to compile, run and clean using the following commands:

**Secondary Hash Table** <br/>

```bash
$ make sht
$ make runsht
$ make clean
```

## Contributors

- [Ioanna Kontemeniotis](https://github.com/joannakonte)
- [Penny Makri](https://github.com/Makri-Panagoula)

