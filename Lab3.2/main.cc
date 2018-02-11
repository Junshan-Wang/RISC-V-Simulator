#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <fstream>
#include <cstdlib>
#include <stdint.h>

using namespace std;

#define memory_latency 100

void InitStats(StorageStats &s);
void PrintStats(StorageStats s);
void execute(char *filename, int size, int block_size, int associativity, int write_through, int write_allocate);
void execute_new(char *filename);
void part11(char *filename);
void part12(char *filename);
void part13(char *filename);

double avg_l1_miss_rate=0;
double avg_l2_miss_rate=0;
double avg_AMAT=0;

int main(int argc, char **argv) {
	if(argc!=2){
                printf("illega parameter\n");
        }
	/*part11(argv[1]);
	printf("\n");
	part12(argv[1]);
	printf("\n");*/
	//part13(argv[1]);
	//printf("\n");
	//execute(argv[1],1<<15,1<<10,8,0,1);
	execute_new(argv[1]);

	return 0;
}

void part11(char *filename){
	int size_n=11;
	int block_size_n=5;
	for(int i=0;i<size_n;++i){
		for(int j=0;j<block_size_n;++j)
			execute(filename,1<<(i+15),1<<(j+6),8,1,0);
		printf("\n");
	}
}

void part12(char *filename){
	int size_n=11;
	int associativity_n=6;
	for(int i=0;i<size_n;++i){
		for(int j=0;j<associativity_n;++j)
			execute(filename,1<<(i+15),1<<10,1<<j,1,0);
		printf("\n");
	}
}

void part13(char *filename){
	execute(filename, 1<<15,1<<10,8,0,0);
	execute(filename, 1<<15,1<<10,8,0,1);
	execute(filename, 1<<15,1<<10,8,1,0);
	execute(filename, 1<<15,1<<10,8,1,1);
	printf("\n");
}

void execute(char *filename, int size, int block_size, int associativity, int write_through, int write_allocate){
	// L1 cache
	int size1,block_size1,associativity1,write_through1,write_allocate1,lower1;
	size1=size;
	block_size1=block_size;
	associativity1=associativity;
	write_through1=write_through;
	write_allocate1=write_allocate;

	// L2 cache
	int size2,block_size2,associativity2,write_through2,write_allocate2,lower2;
	size2=(1<<18);
	block_size2=64;
	associativity2=8;
	write_through2=0;
	write_allocate2=0;

  	Memory *m=new Memory();
  	Cache *l1=new Cache(size1,block_size1,associativity1,write_through1,write_allocate1,m,0,0);

  	StorageStats s;
  	InitStats(s);
  	m->SetStats(s);
  	l1->SetStats(s);

  	StorageLatency ml;
  	ml.bus_latency = 6;
  	ml.hit_latency = 100;
  	m->SetLatency(ml);

  	StorageLatency ll;
  	ll.bus_latency = 3;
  	ll.hit_latency = 1;
  	l1->SetLatency(ll);

	ifstream fin;
	fin.open(filename,ios::in);
	char q;
	uint64_t address;
        char content[1000];
	while(fin>>q){
		fin>>address;
		if(q=='r'){
			l1->HandleRequest(address,1,1,content);
		}
		else if(q=='w'){
			l1->HandleRequest(address,1,0,content);
		}
		else{
			printf("Bad request!\n");
		}
	}

	//l1->GetStats(s);
	//printf("%f ",((double)s.miss_num)/s.access_counter);
	int time=0;
	printf("\n--------L1 cache:--------\n");
  	l1->GetStats(s);
	PrintStats(s);
	time+=s.access_time;

	printf("\n--------Memory cache:--------\n");
  	m->GetStats(s);
	PrintStats(s);
	time+=s.access_time;

	printf("\n-----------------------\n");	
  	printf("Request access time: %dns\n", time);
	

	delete l1;
	delete m;
}


void execute_new(char *filename){
	// L1 cache
	int size1,block_size1,associativity1,write_through1,write_allocate1,lower1;
	size1=(1<<15);
	block_size1=(1<<6);
	associativity1=(1<<3);
	write_through1=0;
	write_allocate1=1;

	// L2 cache
	int size2,block_size2,associativity2,write_through2,write_allocate2,lower2;
	size2=(1<<18);
	block_size2=(1<<6);
	associativity2=(1<<3);
	write_through2=0;
	write_allocate2=1;

  	Memory *m=new Memory();
	Cache *l2=new Cache(size2,block_size2,associativity2,write_through2,write_allocate2,m,0,1);
  	Cache *l1=new Cache(size1,block_size1,associativity1,write_through1,write_allocate1,l2,1,1);

  	StorageStats s;
  	InitStats(s);
  	m->SetStats(s);
  	l1->SetStats(s);
	l2->SetStats(s);

  	StorageLatency ml;
  	ml.bus_latency = 0;
  	ml.hit_latency = 100;
  	m->SetLatency(ml);

  	StorageLatency ll2;
  	ll2.bus_latency = 6;
  	ll2.hit_latency = 4;
  	l2->SetLatency(ll2);
  	
	StorageLatency ll1;
  	ll1.bus_latency = 0;
  	ll1.hit_latency = 3;
  	l1->SetLatency(ll1);

	for(int i=0;i<10;++i){
	ifstream fin;
	fin.open(filename,ios::in);
	char q;
	char s_address[1000];
	uint64_t address;
        char content[1000];
	while(fin>>q){
		//fin>>address;
		fin>>s_address;
		sscanf(s_address,"0x%lx",&address);
		//printf("%ld\n",address);
		if(q=='r'){
			l1->HandleRequest(address,1,1,content);
		}
		else if(q=='w'){
			l1->HandleRequest(address,1,0,content);
		}
		else{
			printf("Bad request! %ld\n",address);
		}
	}

	
	double AMAT=0;
	printf("\n--------Memory:--------\n");
  	m->GetStats(s);
	PrintStats(s);
	AMAT=ml.hit_latency;

	printf("\n--------L2 cache:--------\n");
  	l2->GetStats(s);
	PrintStats(s);
	AMAT=(ll2.hit_latency+ll2.bus_latency+((double)s.miss_num)/s.access_counter*AMAT);
	avg_l2_miss_rate+=((double)s.miss_num)/s.access_counter;

	printf("\n--------L1 cache:--------\n");
  	l1->GetStats(s);
	PrintStats(s);
	AMAT=(ll1.hit_latency+ll1.bus_latency+((double)s.miss_num)/s.access_counter*AMAT);
	avg_l1_miss_rate+=((double)s.miss_num)/s.access_counter;

	printf("\n-----------------------\n");	
  	printf("AMAT: %fns\n", AMAT*clock_cycle);
  	//printf("Request access time: %dns\n", time);

	avg_AMAT+=AMAT*clock_cycle;
	}
	
	printf("l1 miss rate: %f, l2 miss rate: %f, AMAT: %f\n", avg_l1_miss_rate/10, avg_l2_miss_rate/10, avg_AMAT/10);

	delete l1;
	delete l2;
	delete m;
}


void InitStats(StorageStats &s){
	s.access_counter=0;
	s.miss_num=0;
	s.access_time=0;
	s.replace_num=0;
	s.fetch_num=0;
	s.prefetch_num=0;

}

void PrintStats(StorageStats s){
	printf("access_counter: %d\n", s.access_counter);
    	printf("miss_num: %d\n", s.miss_num);
	printf("miss_rate: %f\n", ((double)s.miss_num)/s.access_counter);
    	printf("access_time: %f\n", s.access_time);
    	printf("replace_num: %d\n", s.replace_num);
    	printf("fetch_num: %d\n", s.fetch_num);
    	printf("prefetch_num: %d\n", s.prefetch_num);
}
