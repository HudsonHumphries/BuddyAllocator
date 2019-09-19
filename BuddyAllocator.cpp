#include "BuddyAllocator.h"
#include <iostream>
#include <cmath>
using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size;
  total_memory_size = _total_memory_length;
	start = new char[_total_memory_length];
  int k = log2(double(_total_memory_length)/_basic_block_size);
  cout << k << endl;
  BlockHeader* totalMem = (BlockHeader *) start;
  totalMem->block_size = _total_memory_length;
  totalMem->next = NULL;
  totalMem->free = true;
  FreeList.resize(k+1);
  FreeList[k].insert(totalMem);

  for(int i = 0; i < k; i++) {
    FreeList[i].head = nullptr;
  }
}

BuddyAllocator::~BuddyAllocator (){
	delete start;
}

char* BuddyAllocator::alloc(int _length) {
  int realLength = _length + sizeof(BlockHeader);  
  int index = findindex(realLength);

  if(FreeList[index].head) {
    BlockHeader* b = FreeList[index].head;
    FreeList[index].remove(b);
    return (char *) (b)+1;
  }
  else {
    //we need to go through and split until we can get a block of that size.
    int j = index;
    int k = log2(double(total_memory_size)/basic_block_size);
    //Looking for the smallest block that we can split to get to size of index
    while(j <= k && FreeList[j].head == nullptr) {
      j++;
    }
    //If we looked through all possible bigger block sizes and there are none, we can't give user any memory
    if(j > k) {return nullptr;}

    while(j > index) {
      BlockHeader* b = FreeList[j].head;
      BlockHeader* buddy = split(b);
      FreeList[j].remove(b);
      FreeList[j-1].insert(b);
      FreeList[j-1].insert(buddy);
      j--;
    }
    BlockHeader* final = FreeList[index].head;
    FreeList[index].remove(final);

    return (char*) (final)+1;
  }
  
  return nullptr;
}

int BuddyAllocator::free(char* _a) {
  BlockHeader* test = (BlockHeader *) (_a-1);
  cout << "Free: " << test->block_size << endl;
  int index = findindex(test->block_size);
  FreeList[index].insert(test);
  int i = 0;
  int k = log2(double(total_memory_size)/basic_block_size);
  while(i < k) {
    if(FreeList[i].head) {
      //cout << FreeList[i].head->block_size << endl;
      BlockHeader* headBlock = FreeList[i].head;
      BlockHeader* testBuddy = getbuddy(headBlock);
      cout << "buddy size: " << testBuddy->block_size << endl;
      BlockHeader* mergedBlock = merge(headBlock,testBuddy);
      FreeList[i].remove(headBlock);
      FreeList[i].remove(testBuddy);
      //cout << mergedBlock->block_size << endl;
      FreeList[i+1].insert(mergedBlock);
    }
    i++;
    //cout << i << endl;
  }
  //cout << mergedBlock->block_size << endl;

}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  for (int i=0; i<FreeList.size(); i++){
    cout << "[" << i <<"] (" << ((1<<i) * basic_block_size) << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList [i].head;
    // go through the list from head to tail and count
    while (b){
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      if (b->block_size != (1<<i) * basic_block_size){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      } 
      b = b->next;
    }
    cout << count << endl;  
  }
}

int BuddyAllocator::findindex(int length) {
  vector<int> possibleSizes;
  possibleSizes.resize(double(log2(total_memory_size/basic_block_size))+1);
  possibleSizes[0] = basic_block_size;
  for(int i = 1; i < possibleSizes.size(); i++) {
    possibleSizes[i] = possibleSizes[i-1] * 2;
  }
  int j = 0;
  while(possibleSizes[j] < length) {
    j++;
  }
  return j;
}

BlockHeader* BuddyAllocator::getbuddy(BlockHeader * addr) {
  // given a block address, this function returns the address of its buddy
  char* blockAddr = (char*) (addr)-1;
  cout << "Original Block Size: " << (uintptr_t)(addr->block_size) << endl;
  char* buddyAddress = (char*)((uintptr_t)(blockAddr-start) ^ (uintptr_t)(addr->block_size) + (uintptr_t)start);
  cout << buddyAddress-start << endl;
  BlockHeader* buddy = (BlockHeader*) (buddyAddress);
  return buddy;
}

bool BuddyAllocator::arebuddies (BlockHeader* block1, BlockHeader* block2) {
  // checks whether the two blocks are buddies are not
  return false;
}

BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2) {
  // this function merges the two blocks returns the beginning address of the merged block
  // note that either block1 can be to the left of block2, or the other way around
  char* address1 = (char*) (block1)-1;
  char* address2 = (char*) (block2)-1;
  BlockHeader* mergedB;
  if(address1-start < address2-start) {
    //Address1 is closer to starting address
    mergedB = (BlockHeader *) address1;
  }
  else if(address2-start < address1-start) {
    //Address2 is closer to starting address
    mergedB = (BlockHeader *) address2;
  }
  mergedB->block_size = block1->block_size + block2->block_size;
  return mergedB;
}

BlockHeader* BuddyAllocator::split (BlockHeader* block) {
  int newSize = block->block_size/2;
  char* blockAddress = (char*) (block)+1;
  char* newblockAddress = blockAddress + newSize;
  BlockHeader* newBlock = (BlockHeader *) newblockAddress;
  block->block_size = newSize;
  newBlock->block_size = newSize;
  return newBlock;
}

