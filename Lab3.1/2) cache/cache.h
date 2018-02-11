#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"

typedef struct CacheConfig_ {
 	int size;
  	int associativity;
 	int set_num; // Number of cache sets
  	int write_through; // 0|1 for back|through
  	int write_allocate; // 0|1 for no-alc|alc
	int block_size;

	int offset_bit;
	int index_bit;
	int tag_bit;
} CacheConfig;

typedef struct Line_ {
	bool valid;
	bool dirty;
	uint64_t tag;
	char *content;
} Line;


class Cache: public Storage {
public:
  	Cache(int size, int block_size, int associativity, int write_through, int write_allocate, Storage *lower);
  	~Cache();

  	// Sets & Gets
  	void SetConfig(CacheConfig cc);
  	void GetConfig(CacheConfig cc);
  	void SetLower(Storage *ll) { lower_ = ll; }
  	// Main access process
  	void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content);

 private:
 	 // Bypassing
  	int BypassDecision();
  	// Partitioning
  	void PartitionAlgorithm();
  	// Replacement
  	int ReplaceDecision(uint64_t index, uint64_t tag);
  	int ReplaceAlgorithm(uint64_t index, uint64_t tag);
	void ReplaceInfoUpdate(uint64_t index, int pos);
  
	// Prefetching
  	int PrefetchDecision();
  	void PrefetchAlgorithm();

  	CacheConfig config_;
  	Storage *lower_;
  	DISALLOW_COPY_AND_ASSIGN(Cache);

	Line **lines;
	int **last_use;
};

#endif //CACHE_CACHE_H_ 
