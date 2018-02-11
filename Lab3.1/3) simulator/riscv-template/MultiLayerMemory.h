#include "cache.h"
#include "memory.h"
#include "storage.h"

class MultiLayerMemory{
	Memory *m;
	Cache *l1;
	Cache *l2;
	Cache *llc;

	StorageStats s;
	StorageLatency ml;
	StorageLatency ll1;
	StorageLatency ll2;
	StorageLatency lllc;

public:
	MultiLayerMemory();
	~MultiLayerMemory();
	void InitStats(StorageStats &s);

	unsigned long long ReadMemory(int addr, int bytes);
	void WriteMemory(int addr, int bytes, unsigned long long num);
	void WriteMemory(int addr, int bytes, char *content);
	int AccessCycle();
};
