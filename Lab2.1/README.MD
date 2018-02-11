./samples中为测试代码.
./riscv-template中为模拟器代码，其中设置Simulation.cpp中的debug变量可以实现单步模式，Makefile为编译文件，elf为解析elf得到的文件。
./run.sh为执行脚本，利用RISC-V工具链编译生产ELF文件，然后通过本次Lab实现的模拟器运行。需要一个参数，给出执行的c++代码文件。例：./run.sh add.c
./READ.MD为本文件
./Lab2.1报告为本次Lab的实验报告。

注：若无安装RESC-V工具链无法编译，可以直接在./riscv-template目录下执行./Simulation，将运行矩阵乘法代码。
