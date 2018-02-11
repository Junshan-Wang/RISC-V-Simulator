cd samples
riscv64-unknown-elf-gcc -Wa,-march=rv64im -o a.out $1
cd ..
cd riscv-template
make
./Simulation
cd ..
