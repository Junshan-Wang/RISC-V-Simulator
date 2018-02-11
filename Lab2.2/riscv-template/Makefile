CC=g++

Simulation: Simulation.o Read_Elf.o
	$(CC) -o Simulation Simulation.o Read_Elf.o
Simulation.o: Simulation.cpp Simulation.h Reg_def.h
	$(CC) -c Simulation.cpp
Read_Elf.o: Read_Elf.cpp Read_Elf.h
	$(CC) -c Read_Elf.cpp

clean:
	rm -fr *.o Simulation
