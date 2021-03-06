#include<iostream>
#include<stdio.h>
#include<math.h>
#include <io.h>
#include <process.h>
#include<time.h>
#include<stdlib.h>
#include"Reg_def.h"


#define OP_JAL 111//0x6f
#define OP_R 51 //0x33

#define F3_ADD 0
#define F3_MUL 0

#define F7_MSE 1
#define F7_ADD 0

#define OP_I 19 //0x13
#define F3_ADDI 0

#define OP_SW 35 //0x23
#define F3_SB 0

#define OP_LW 3    //0x3
#define F3_LB 0

#define OP_BEQ 99 //0x63
#define F3_BEQ 0

#define OP_IW 27 //0x1B
#define F3_ADDIW 0

#define OP_RW 59 //0x3B
#define F3_ADDW 0
#define F7_ADDW 0


#define OP_SCALL 115 //0x73
#define F3_SCALL 0
#define F7_SCALL 0

#define OP_JALR 103 //0x67
#define OP_AUIPC 23 //0x17
#define OP_LUI 55 //0x37


#define MAX 100000000

//主存
unsigned int memory[MAX]={0};
//寄存器堆
REG reg[32]={0};
//PC
int PC=0;


//各个指令解析段
unsigned int OP=0;
unsigned int fuc3=0,fuc7=0;
int shamt=0;
int rs=0,rt=0,rd=0;
unsigned int imm12=0;
unsigned int imm20=0;
unsigned int imm7=0;
unsigned int imm5=0;



//加载内存
void load_memory();

void simulate_simple();
void simulate_multiple();
void simulate_pipeline();

void IF();

void ID();

void EX();

void MEM();

void WB();


//符号扩展
int ext_signed(unsigned int src,int bit);

//获取指定位
unsigned int getbit(int s,int e);

unsigned int getbit(int inst,int s,int e)
{
	unsigned int ins=inst;
	ins=((ins<<(32-e))>>(32-e+s));
	return ins;
}

int ext_signed(unsigned int src,int bit)
{
	int dst=src;
	if((src>>(bit-1))&1){
		dst=(dst<<(32-bit))>>(32-bit);
	}
    	return dst;
}

void PrintStatus();
bool DataHazards();
void nop();
void InitializePipeline();
void cover();

