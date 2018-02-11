CC=g++

Simulation: Simulation.o Read_Elf.o MultiLayerMemory.o cache.o memory.o
	$(CC) -o Simulation Simulation.o Read_Elf.o MultiLayerMemory.o cache.o memory.o
Simulation.o: Simulation.cpp Simulation.h Reg_def.h MultiLayerMemory.h
	$(CC) -c Simulation.cpp
Read_Elf.o: Read_Elf.cpp Read_Elf.h
	$(CC) -c Read_Elf.cpp
MultiLayerMemory.o: MultiLayerMemory.cpp MultiLayerMemory.h cache.h memory.h
	$(CC) -c MultiLayerMemory.cpp
cache.o: cache.h def.h
memory.o: memory.h

clean: 
	rm -fr *.o Simulation

