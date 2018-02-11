#include "MultiLayerMemory.h"

MultiLayerMemory::MultiLayerMemory(){
		m=new Memory();
		llc=new Cache(1<<23,1<<6,1<<3,0,1,m);
		l2=new Cache(1<<18,1<<6,1<<3,0,1,llc);
		l1=new Cache(1<<15,1<<6,1<<3,0,1,l2);
        	
		InitStats(s);
        	m->SetStats(s);
        	l1->SetStats(s);
        	l2->SetStats(s);
        	llc->SetStats(s);

        	ml.bus_latency = 6;
        	ml.hit_latency = 100;
        	m->SetLatency(ml);

        	lllc.bus_latency = 3;
        	lllc.hit_latency = 1;
        	llc->SetLatency(lllc);

        	ll2.bus_latency = 3;
        	ll2.hit_latency = 8;
        	l2->SetLatency(ll2);

        	ll1.bus_latency = 3;
        	ll1.hit_latency = 20;
        	l1->SetLatency(ll1);

}

MultiLayerMemory::~MultiLayerMemory(){
		delete l1;
		delete l2;
		delete llc;
		delete m;
}

void MultiLayerMemory::InitStats(StorageStats &s){
        	s.access_counter=0;
        	s.miss_num=0;
        	s.access_time=0;
        	s.replace_num=0;
        	s.fetch_num=0;
        	s.prefetch_num=0;
}

unsigned long long MultiLayerMemory::ReadMemory(int addr, int bytes){
		char *content=new char[bytes+1];
		for(int i=0;i<bytes;++i)
			l1->HandleRequest(addr+i,1,1,content+i);
		unsigned long long num=*((unsigned long long*)content);
		delete content;
		return num;
}

void MultiLayerMemory::WriteMemory(int addr, int bytes, unsigned long long num){
		char *content=(char*)&num;
		for(int i=0;i<bytes;++i)
			l1->HandleRequest(addr+i,1,0,content+i);
}

void MultiLayerMemory::WriteMemory(int addr, int bytes, char *content){
		for(int i=0;i<bytes;++i)
			l1->HandleRequest(addr+i,1,0,content+i);
}

int MultiLayerMemory::AccessCycle(){
	double time=0;
	StorageStats tmps;
	l1->GetStats(tmps);
	time+=tmps.access_time;
	l2->GetStats(tmps);
	time+=tmps.access_time;
	llc->GetStats(tmps);
	time+=tmps.access_time;
	m->GetStats(tmps);
	time+=tmps.access_time;
	return time/clock_cycle;
}
