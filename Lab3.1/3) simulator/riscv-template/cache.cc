#include "cache.h"
#include "def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int debug1=0;

int GetBitNum(int num){
	int bit=0;
	while(num!=1){
		if(num&1!=0){
			printf("input not 2^x\n");
			exit(0);
		}
		num=num>>1;
		bit++;
	}
	return bit;
}


Cache::Cache(int size, int block_size, int associativity, 
	int write_through, int write_allocate, Storage *lower){
	config_.size=size;
	config_.associativity=associativity;
	config_.set_num=size/associativity/block_size;
	config_.write_through=write_through;
	config_.write_allocate=write_allocate;
	config_.block_size=block_size;
	config_.offset_bit=GetBitNum(block_size);
	config_.index_bit=GetBitNum(config_.set_num);
	config_.tag_bit=64-config_.offset_bit-config_.index_bit;

	if(debug1) printf("Cache Initialize with size %d, associatovity %d, set_num %d, block_size %d\n", size, associativity, config_.set_num, block_size);

	lower_=lower;
	lines=new Line*[config_.set_num];
	last_use=new int*[config_.set_num];
	for(int i=0;i<config_.set_num;++i){
		lines[i]=new Line[config_.associativity];
		last_use[i]=new int[config_.associativity];
		for(int j=0;j<config_.associativity;++j){
			lines[i][j].valid=FALSE;
			lines[i][j].dirty=FALSE;
			lines[i][j].tag=0;
			lines[i][j].content=new char[config_.block_size];
			last_use[i][j]=0;
		}
	}

	if(debug1) printf("Cache construct succeed\n");
}

Cache::~Cache(){
	for(int i=0;i<config_.set_num;++i){
		for(int j=0;j<config_.associativity;++j){
			delete lines[i][j].content;
		}
		delete []last_use[i];
		delete lines[i];
	}
	delete lines;
	delete last_use;
}

void Cache::HandleRequest(uint64_t addr, int bytes, int read, char *content) {

	uint64_t offset=addr&((1<<config_.offset_bit)-1);
	uint64_t index=(addr>>config_.offset_bit)&((1<<config_.index_bit)-1);
	uint64_t tag=(addr>>(config_.offset_bit+config_.index_bit))&((1<<config_.tag_bit)-1);

	if(debug1) printf("Start cache request with addr: %lx, bytes: %d, offset: %lx, index: %lx, tag: %lx\n", addr, bytes, offset, index, tag);

	if(offset+bytes>config_.block_size){
		printf("content not in a single block\n");
		exit(0);
	}

	// Bypass?
	if (!BypassDecision()) {
    		// Miss?
		int pos=ReplaceDecision(index,tag);
		if(debug1) printf("Hit position: %d\n",pos);
		if(read==1){
			if(pos>=0){
				if(debug1) printf("Read Hit\n");
				for(int i=0;i<bytes;++i)
                                	content[i]=lines[index][pos].content[offset+i];
			}
			else{
				if(debug1) printf("Read Miss\n");
				pos=ReplaceAlgorithm(index,tag);
        	
				char *lower_content=new char[config_.block_size];
        			lower_->HandleRequest(addr-offset,config_.block_size,1,lower_content);
				stats_.fetch_num++;

        			for(int i=0;i<config_.block_size;++i)
                			lines[index][pos].content[i]=lower_content[i];
        			lines[index][pos].valid=TRUE;
        			lines[index][pos].dirty=FALSE;
       		 		lines[index][pos].tag=tag;
				delete lower_content;				

				for(int i=0;i<bytes;++i)
                                	content[i]=lines[index][pos].content[offset+i];

				stats_.miss_num++;
			}
			ReplaceInfoUpdate(index,pos);
		}
		else{
			if(pos>=0){
				if(debug1) printf("Write Hit\n");
				for(int i=0;i<bytes;++i)
					lines[index][pos].content[offset+i]=content[i];
				lines[index][pos].valid=TRUE;
				lines[index][pos].dirty=FALSE;
				lines[index][pos].tag=tag;
				if(config_.write_through==1){
					lower_->HandleRequest(addr,bytes,0,content);
					stats_.fetch_num++;
				}
				else
					lines[index][pos].dirty=TRUE;
				ReplaceInfoUpdate(index,pos);
			}
			else{
				if(debug1) printf("Write Miss\n");
				if(config_.write_allocate==1){
					pos=ReplaceAlgorithm(index,tag);
					if(debug1) printf("Out replace\n");
					if(debug1) printf("content %c\n",content[0]);
					for(int i=0;i<bytes;++i)
						lines[index][pos].content[offset+i]=content[i];
					lines[index][pos].valid=TRUE;
					lines[index][pos].dirty=FALSE;
					lines[index][pos].tag=tag;
					if(config_.write_through==1){
						lower_->HandleRequest(addr,bytes,0,content);
						stats_.fetch_num++;
					}
					else
						lines[index][pos].dirty=TRUE;	
					ReplaceInfoUpdate(index,pos);
				}
				else{
					lower_->HandleRequest(addr,bytes,0,content);
					stats_.fetch_num++;
				}

				stats_.miss_num++;
			}
		}
		stats_.access_counter++;
		stats_.access_time+=latency_.hit_latency*clock_cycle;
  	}

  	// Prefetch?
  	/*if (PrefetchDecision()) {
    		PrefetchAlgorithm();
  	} 
	else {
    		// Fetch from lower layer
    		int lower_hit, lower_time;
    		lower_->HandleRequest(addr, bytes, read, content,
                          lower_hit, lower_time);
   		hit = 0;
    		time += latency_.bus_latency + lower_time;
    		stats_.access_time += latency_.bus_latency;
 	}*/

	if(debug1) printf("End cache request\n");
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm() {
}

int Cache::ReplaceDecision(uint64_t index, uint64_t tag) {
	if(debug1) printf("Start replace decision %d %ld %ld\n",config_.associativity,index,tag);
	for(int i=0;i<config_.associativity;++i){
		if(lines[index][i].tag==tag && lines[index][i].valid==TRUE){
			return i;
		}
	}
	return -1;
}

int Cache::ReplaceAlgorithm(uint64_t index, uint64_t tag){
	if(debug1) printf("Begin replace algorithm\n");	
	// Get the replaced position in the current cache
	int pos=0, max_count=0;
	for(int i=0;i<config_.associativity;++i){
		if(last_use[index][i]>max_count){
			pos=i;
			max_count=last_use[index][i];
		}
	}

	if(debug1) printf("Replace %d of %ld: valid %d\n",pos,index,lines[index][pos].valid);

	// Write back
	if(config_.write_through==0 && lines[index][pos].valid==TRUE && lines[index][pos].dirty==TRUE){
		uint64_t lower_addr=(lines[index][pos].tag<<(config_.offset_bit+config_.index_bit))+(index<<(config_.offset_bit));
		if(debug1) printf("Before lower request\n");
		lower_->HandleRequest(lower_addr,config_.block_size,0,lines[index][pos].content);
		stats_.fetch_num++;
	}
	
	stats_.replace_num++;
	if(debug1) printf("Exit replace\n");
	return pos;
}

void Cache::ReplaceInfoUpdate(uint64_t index, int pos){
	for(int i=0;i<config_.associativity;++i){
		last_use[index][i]++;
	}
	last_use[index][pos]=0;

	if(debug1) printf("Hit %d of %ld\n",pos,index);
}


int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

