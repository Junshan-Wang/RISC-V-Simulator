#include "memory.h"

int debug2=0;

Memory::Memory(){
	mem=new char[MEMSIZE];
	memset(mem,0,MEMSIZE);
	if(debug2) printf("Memory construct succeed\n");
}

Memory::~Memory(){
	delete mem;
}

void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content) {
	if(debug2) printf("Start memory request with addr: %lx, bytes: %d\n", addr, bytes);
	
	if(addr>MEMSIZE){
		if(debug2) printf("Bad memory address!\n");
		memset(content,0,bytes);
		return;
	}

	if(read==1){
		memcpy(content,mem+addr,bytes);
	}
	else{
		memcpy(mem+addr,content,bytes);
	}
  
	stats_.access_counter++;
	stats_.access_time+=latency_.hit_latency*clock_cycle;

	if(debug2) printf("End memory request\n");
}

