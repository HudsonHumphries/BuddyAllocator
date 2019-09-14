#include "BuddyAllocator.h"
#include <iostream>
#include <cmath>
using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size;
  total_memory_size = _total_memory_length;
	start = new char[_total_memory_length];
  int k = log2(double(_total_memory_length)/_basic_block_size);
  BlockHeader* totalMem = (BlockHeader *) start;
  totalMem->block_size = _total_memory_length;
  totalMem->next = NULL;
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
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  return new char [_length];
}

int BuddyAllocator::free(char* _a) {
  /* Same here! */
  delete _a;
  return 0;
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

