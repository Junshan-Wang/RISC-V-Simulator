#!/bin/bash
cd samples
riscv64-unknown-elf-gcc -Wa,-march=rv64im -o a.out $1
cd ..
cd riscv-template 
make

#if [$# == 1]
#then
#	./Simulation 0
#else
./Simulation $2 $3
#fi

cd ..
