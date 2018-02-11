#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"
#include <elf.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MEMSIZE 0x10000000

class Memory: public Storage {
public:
  	Memory();
  	~Memory();

  	// Main access process
  	void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content);

 	private:
	char *mem;
  	// Memory implement
  	DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 
