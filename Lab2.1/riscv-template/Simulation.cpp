#include "Simulation.h"
using namespace std;

extern void read_elf();
extern unsigned int cadr;
extern unsigned int csize;
extern unsigned int vadr;
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int endPC;
extern unsigned int entry;
extern FILE *file;

extern unsigned int data_adr;
extern unsigned int data_vadr;
extern unsigned int data_size;

extern unsigned int sdata_adr;
extern unsigned int sdata_vadr;
extern unsigned int sdata_size;

//单步模式
int debug=0;

//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;

//加载代码段
//初始化PC
void load_memory()
{
	//printf("code address: %x,size: %x\n",vadr>>2,csize);
	//printf("file offset: %x\n",cadr);

	fseek(file,cadr,SEEK_SET);
	fread(&memory[vadr>>2],1,csize,file);
	vadr=vadr>>2;
	csize=csize>>2;

	fseek(file,data_adr,SEEK_SET);
	fread(&memory[data_vadr>>2],1,data_size,file);
	data_vadr=data_vadr>>2;
	data_size=data_size>>2;

	fseek(file,sdata_adr,SEEK_SET);
        fread(&memory[sdata_vadr>>2],1,sdata_size,file);
        sdata_vadr=sdata_vadr>>2;
        sdata_size=sdata_size>>2;

	fclose(file);
}

int main()
{
	//cout<<"compile succeed!"<<endl;

	file=fopen("../samples/a.out","r");	

	//解析elf文件
	read_elf();
	
	//加载内存
	load_memory();

	//设置入口地址
	PC=entry>>2;
	PC=madr;	
	//设置全局数据段地址寄存器
	reg[3]=gp;
	
	reg[2]=MAX/2;//栈基址 （sp寄存器）
	//reg[2]=data_vadr+32;

	simulate();

	cout<<"simulate over!"<<endl;
	cout<<"Display Status?(y/n)";
	char *c=new char[100];
	cin>>c;
	if(c[0]=='y')
		PrintStatus();

	return 0;
}

void simulate()
{
	//结束PC的设置
	int end=(int)endPC/4-1;
	while(PC!=0)
	{
		//运行
		IF();IF_ID=IF_ID_old;
		ID();ID_EX=ID_EX_old;
		EX();EX_MEM=EX_MEM_old;
		MEM();MEM_WB=MEM_WB_old;
		WB();

		//更新中间寄存器
		//IF_ID=IF_ID_old;
		//ID_EX=ID_EX_old;
		//EX_MEM=EX_MEM_old;
		//MEM_WB=MEM_WB_old;

        	if(exit_flag==1)
            		break;

        	reg[0]=0;//一直为零

        	if(debug){
        		PrintStatus();
        	}
	}
}

void PrintStatus()
{
	/* PC */
	printf("PC: \n0x%x\n",PC);
	/* Register */
	cout<<"-----Register-----"<<endl;
	for(int i=0;i<32;++i){
		printf("%d:%llx; ",i,reg[i]);
		//cout<<i<<":"<<reg[i]<<";";
	}
	cout<<endl;
	/* Memory */
	cout<<"------Memory: data------"<<endl;
	//cout<<data_vadr<<" "<<data_size<<endl;
	printf("data_vadr:%x, data_size:%x\n",data_vadr<<2,data_size<<2);
	for(int i=data_vadr;i<data_vadr+10;i+=1){
		if(i<MAX) cout<<memory[i]<<" ";
		else { cout<<"out of memory"; break;}
	}
	cout<<endl;
	
	/*cout<<"------Memory: bss------"<<endl;
	//cout<<sdata_vadr<<" "<<sdata_size<<endl;
	printf("bss_vadr:%x, bss_size:%x\n",sdata_vadr,sdata_size<<2);
	for(int i=sdata_vadr;i<sdata_vadr+10;i+=1){
		if(i<MAX) cout<<memory[i]<<" ";
		else { cout<<"out of memory"; break;}
	}
	cout<<endl;
	
	cout<<"------Memory: gp------"<<endl;
	//cout<<gp<<endl;
	printf("gp:%llx\n",gp);
	for(int i=(gp>>2);i<(gp>>2)+10;i+=1){
		if(i<MAX) cout<<memory[i]<<" ";
		else { cout<<"out of memory"; break;}
	}
	cout<<endl;*/

	/*cout<<"------Stack------"<<endl;
        for(int i=(reg[2]);i<(reg[2])+100;i+=1){
                if(i<MAX) cout<<memory[i]<<" ";
                else { cout<<"out of memory"; break;}
        }
        cout<<endl;*/

	/*while(1){
		long long adr=-1;
		cout<<"Enter memory address: ('-1' to continue) ";
		cin>>adr;
		if(adr==-1)
			break;
		if(adr<MAX)
			cout<<"memory of address "<<adr<<": "<<memory[adr]<<endl;
		else
			cout<<"Out of bound"<<endl;
	}*/
	

	//cin.get();
	//cout<<endl<<"========== NEXT =========="<<endl;



	/*cout<<"IF_ID "<<IF_ID.inst<<" "<<IF_ID.PC<<endl;
	cout<<"ID_EX "<<ID_EX.Rd<<" "<<ID_EX.Rt<<" "<<ID_EX.PC<<" "<<ID_EX.Imm<<" "<<ID_EX.Reg_Rs<<" "<<ID_EX.Reg_Rt<<" "<<(int)ID_EX.Ctrl_EX_ALUSrc<<" "<<(int)ID_EX.Ctrl_EX_ALUOp<<" "<<(int)ID_EX.Ctrl_EX_RegDst<<" "<<(int)ID_EX.Ctrl_M_Branch<<" "<<(int)ID_EX.Ctrl_M_MemWrite<<" "<<(int)ID_EX.Ctrl_M_MemRead<<" "<<(int)ID_EX.Ctrl_WB_RegWrite<<" "<<(int)ID_EX.Ctrl_WB_MemtoReg<<endl;
	cout<<"EX_MEM "<<PC<<" "<<EX_MEM.Reg_dst<<" "<<EX_MEM.ALU_out<<" "<<EX_MEM.Zero<<" "<<EX_MEM.Reg_Rt<<" "<<(int)EX_MEM.Ctrl_M_Branch<<" "<<(int)EX_MEM.Ctrl_M_MemWrite<<" "<<(int)EX_MEM.Ctrl_M_MemRead<<" "<<(int)EX_MEM.Ctrl_WB_RegWrite<<" "<<(int)EX_MEM.Ctrl_WB_MemtoReg<<endl;
	cout<<"MEM_WB "<<MEM_WB.Mem_read<<" "<<MEM_WB.ALU_out<<" "<<MEM_WB.Reg_dst<<" "<<(int)MEM_WB.Ctrl_WB_RegWrite<<" "<<(int)MEM_WB.Ctrl_WB_MemtoReg<<endl;
	*/
	cin.get();
	cout<<endl<<"========== NEXT =========="<<endl;
}


//取指令
void IF()
{
	//write IF_ID_old
	IF_ID_old.inst=memory[PC>>2];
	PC=PC+4;
	IF_ID_old.PC=PC;
}

//译码
void ID()
{
	//Read IF_ID
	/*
	EXTop: extender control
	EXTsrc: source value of extender
	RegDst: choose the destination register if write back
	ALUop: operator of ALU
	ALUSrc: choose source of ALU
	Branch?
	MemRead/MemWrite: control read/write memory
	RegWrite: control register write back
	MemtoReg: choose memory or ALU to result
	*/

	unsigned int inst=IF_ID.inst;
	int EXTop=0;
	unsigned int EXTsrc=0;

	char RegDst,ALUOp,ALUSrc;
	char Branch,MemRead,MemWrite;
	char RegWrite,MemtoReg;

	rd=getbit(inst,7,12);
	fuc3=getbit(inst,0,0);

	OP=getbit(inst,0,7);
	fuc3=getbit(inst,12,15);
	fuc7=getbit(inst,25,32);
	shamt=0;
	rs=getbit(inst,15,20);
	rt=getbit(inst,20,25);
	imm12=getbit(inst,20,32);
	imm7=getbit(inst,25,32);
	imm5=getbit(inst,7,12);
	imm20=getbit(inst,12,32);

	//if(debug) printf("inst: %x, op:%x\n",inst,OP);
	
	if(OP==OP_R || OP==OP_R_W)
	{
		switch(fuc3){
		case 0:
			switch(fuc7){
			case 0:
				ALUOp=0;
				if(debug) printf("add %d,%d,%d\n",rd,rs,rt);
				break;
			case 1:
				ALUOp=1;
				if(debug) printf("mul %d,%d,%d\n",rd,rs,rt);
				break;
			case 32:
				ALUOp=2;
				if(debug) printf("sub %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 1:
			switch(fuc7){
			case 0:
				ALUOp=3;
				if(debug) printf("sll %d,%d,%d\n",rd,rs,rt);
				break;
			case 1:
				ALUOp=4;
				if(debug) printf("mulh %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 2:
			switch(fuc7){
			case 0:
				ALUOp=5;
				if(debug) printf("slt %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator!  PC: %x\n", PC);break;
			}break;
		case 4:
			switch(fuc7){
			case 0:
				ALUOp=6;
				if(debug) printf("xor %d,%d,%d\n",rd,rs,rt);
				break;
			case 1:
				ALUOp=7;
				if(debug) printf("div %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 5:
			switch(fuc7){
			case 0:
				ALUOp=8;
				if(debug) printf("srl %d,%d,%d\n",rd,rs,rt);
				break;
			case 32:
				ALUOp=9;
				if(debug) printf("sra %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 6:
			switch(fuc7){
			case 0:
				ALUOp=10;
				if(debug) printf("or %d,%d,%d\n",rd,rs,rt);
				break;
			case 1:
				ALUOp=11;
				if(debug) printf("rem %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 7:
			switch(fuc7){
			case 0:
				ALUOp=12;
				if(debug) printf("and %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				printf("Bad operator! PC: %x\n", PC);break;
			}break;
		}
        	EXTop=0;
        	EXTsrc=0;
		RegDst=1;
		ALUSrc=0;
		Branch=0;
		MemRead=0;
		MemWrite=0;
		RegWrite=1;
		MemtoReg=0;
	}
	else if(OP==OP_I || OP==OP_I_W)
    	{
        if(fuc3==0){
        	EXTsrc=imm12;
		ALUOp=0;
		if(debug) printf("addi %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==1 && fuc7==0){
           	EXTsrc=getbit(imm12,0,5);
		ALUOp=3;
		if(debug) printf("slli %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==2){
        	EXTsrc=imm12;
		ALUOp=5;
		if(debug) printf("slti %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==4){
           	EXTsrc=imm12;
		ALUOp=6;
		if(debug) printf("xori %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==5 && fuc7==0){
           	EXTsrc=getbit(imm12,0,5);
		ALUOp=8;
		if(debug) printf("srli %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==5 && fuc7==32){
           	EXTsrc=getbit(imm12,0,5);
		ALUOp=9;
		if(debug) printf("srai %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==6){
           	EXTsrc=imm12;
		ALUOp=10;
		if(debug) printf("ori %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else if(fuc3==7){
           	EXTsrc=imm12;
		ALUOp=12;
		if(debug) printf("andi %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
        }
        else{
        	printf("Bad operator! PC: %x\n", PC);
        }
        EXTop=12;
        RegDst=1;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;
    	}
    	else if(OP==OP_SW)
    	{
        if(fuc3==0){
           	MemWrite=1;
           	if(debug) printf("sb %d,%d(%d)\n",rt,ext_signed((imm7<<5)+imm5,12),rs);
        }
        else if(fuc3==1){
           	MemWrite=2;
           	if(debug) printf("sh %d,%d(%d)\n",rt,ext_signed((imm7<<5)+imm5,12),rs);
        }
        else if(fuc3==2){
        	MemWrite=3;
        	if(debug) printf("sw %d,%d(%d)\n",rt,ext_signed((imm7<<5)+imm5,12),rs);
        }
        else if(fuc3==3){
        	MemWrite=4;
        	if(debug) printf("sd %d,%d(%d)\n",rt,ext_signed((imm7<<5)+imm5,12),rs);
        }
        else{
        	printf("Bad operator! PC: %x\n", PC);
        }
        EXTop=12;
        EXTsrc=(imm7<<5)+imm5;
        RegDst=0;
        ALUOp=0;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        RegWrite=0;
        MemtoReg=0;
    	}
    	else if(OP==OP_LW)
    	{
        if(fuc3==0){
           	MemRead=1;
           	if(debug) printf("lb %d,%d(%d)\n",rd,ext_signed(imm12,12),rs);
        }
        else if(fuc3==1){
           	MemRead=2;
           	if(debug) printf("lh %d,%d(%d)\n",rd,ext_signed(imm12,12),rs);
        }
        else if(fuc3==2){
        	MemRead=3;
        	if(debug) printf("lw %d,%d(%d)\n",rd,ext_signed(imm12,12),rs);
        }
        else if(fuc3==3){
        	MemRead=4;
        	if(debug) printf("ld %d,%d(%d)\n",rd,ext_signed(imm12,12),rs);
        }
        else{
        	printf("Bad operator! PC: %x\n", PC);
        }
        EXTop=12;
        EXTsrc=imm12;
        RegDst=1;
        ALUOp=0;
        ALUSrc=1;
        Branch=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=1;
    	}
    	else if(OP==OP_BEQ)
    	{
	EXTop=13;
	EXTsrc=((imm7&0x40)<<6)+((imm5&0x1)<<11)+((imm7&0x3f)<<5)+((imm5&0x1e));
        if(fuc3==0){
		Branch=1;
		if(debug) printf("beq %d,%d,%d\n",rs,rt,ext_signed(EXTsrc,EXTop));
        }
        else if(fuc3==1){
		Branch=2;
		if(debug) printf("bne %d,%d,%d\n",rs,rt,ext_signed(EXTsrc,EXTop));
        }
        else if(fuc3==4){
		Branch=3;
		if(debug) printf("blt %d,%d,%d\n",rs,rt,ext_signed(EXTsrc,EXTop));
        }
        else if(fuc3==5){
		Branch=4;
		if(debug) printf("bge %d,%d,%d\n",rs,rt,ext_signed(EXTsrc,EXTop));
        }
        else{
           	printf("Bad operator! PC: %x\n", PC);
        }
        //EXTop=12;
        //EXTsrc=(((imm7<<5)+imm5));
        RegDst=1;
        ALUOp=2;
        ALUSrc=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=0;
        MemtoReg=0;
   	}
    	else if(OP==OP_IW)
    	{
    	if(fuc3==0){
    		EXTop=12;
        	EXTsrc=imm12;
        	RegDst=1;
        	ALUOp=0;
        	ALUSrc=1;
        	Branch=0;
        	MemRead=0;
        	MemWrite=0;
        	RegWrite=1;
        	MemtoReg=0;
        	if(debug) printf("addiw %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
    	}
    	else{
    		printf("Bad operator! PC: %x\n", PC);
    	}
    	}
    	else if(OP==OP_JALR)
    	{
    	EXTop=12;
        EXTsrc=imm12;
        RegDst=1;
        ALUOp=15;
        ALUSrc=1;
        Branch=5;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;	
        if(debug) printf("jalr %d,%d,%d\n",rd,rs,ext_signed(imm12,12));
    	}
    	else if(OP==OP_JAL)
    	{
        EXTop=21;
        EXTsrc=((imm20&0x80000)<<1)+((imm20&0xff)<<12)+((imm20&0x100)<<3)+((imm20&0x7fe00)>>8);
        RegDst=1;
        ALUOp=15;
        ALUSrc=1;
        Branch=6;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;	
        if(debug) printf("jal %d,%d\n",rd,ext_signed(EXTsrc,EXTop));
    	}
    	else if(OP==OP_SCALL)
    	{
    	if(debug) printf("ecall\n");
    	}
    	else if(OP==OP_AUIPC)
    	{
    	EXTop=32;
        EXTsrc=(imm20<<12);
        RegDst=1;
        ALUOp=13;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;	
        if(debug) printf("auipc %d,%d\n",rd,ext_signed(imm20,20));
    	}
    	else if(OP==OP_LUI)
    	{
    	EXTop=20;
        EXTsrc=(imm20<<12);
        RegDst=1;
        ALUOp=14;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;	
        if(debug) printf("lui %d,%d\n",rd,ext_signed(imm20,20));
    	}
    	else
    	{
	printf("Bad operator! PC: %x\n", PC);
    	}

	//write ID_EX_old
	ID_EX_old.Rd=rd;
	ID_EX_old.Rt=rt;
	ID_EX_old.PC=PC-4;
	ID_EX_old.Imm=ext_signed(EXTsrc,EXTop);
	ID_EX_old.Reg_Rs=reg[rs];
	ID_EX_old.Reg_Rt=reg[rt];

	ID_EX_old.Ctrl_EX_ALUSrc=ALUSrc;
	ID_EX_old.Ctrl_EX_ALUOp=ALUOp;
	ID_EX_old.Ctrl_EX_RegDst=RegDst;

	ID_EX_old.Ctrl_M_Branch=Branch;
	ID_EX_old.Ctrl_M_MemWrite=MemWrite;
	ID_EX_old.Ctrl_M_MemRead=MemRead;

	ID_EX_old.Ctrl_WB_RegWrite=RegWrite;
	ID_EX_old.Ctrl_WB_MemtoReg=MemtoReg;

}

//执行
void EX()
{
	//read ID_EX
	int tempPC=ID_EX.PC;
	char RegDst=ID_EX.Ctrl_EX_RegDst;
	char ALUOp=ID_EX.Ctrl_EX_ALUOp;
	char ALUSrc=ID_EX.Ctrl_EX_ALUSrc;
	char Branch=ID_EX.Ctrl_M_Branch;

	//Branch PC calulate
	if(Branch==0){
	}
	else if(Branch<5){
		tempPC=(tempPC+ID_EX.Imm);
		//tempPC+=(ID_EX.Imm>>2);
		//tempPC=(tempPC-tempPC%2);
	}
	else if(Branch==5){
		tempPC=(ID_EX.Reg_Rs+ID_EX.Imm);
		//tempPC=ID_EX.Reg_Rs+(ID_EX.Imm>>2);
		tempPC=(tempPC-tempPC%2);
	}
	else{
		tempPC=(tempPC+ID_EX.Imm);
	}

	//choose ALU input number
	REG a=ID_EX.Reg_Rs;
	REG b;
	switch(ALUSrc){
	case 0:
		b=ID_EX.Reg_Rt;break;
	case 1:
		b=ID_EX.Imm;break;
	default:
		printf("Bad ALUSrc!\n");
	}

	//alu calculate
	int Zero;
	REG ALUout;
	switch(ALUOp){
	case 0:
		ALUout=a+b;break;
	case 1:{
		__int128_t tmp_ALUout=a*b;
		ALUout=(REG)tmp_ALUout;
		break;
		}
	case 2:{
		ALUout=a-b;
		if((long long)ALUout<0)
			Zero=-1;
		else if((long long)ALUout>0)
			Zero=1;
		else
			Zero=0;
		break;
		}
	case 3:
		ALUout=a<<b;break;
	case 4:{
		__int128_t tmp_ALUout=a*b;
		ALUout=(REG)(tmp_ALUout>>63);
		break;
		}
	case 5:{
		if(a<b) ALUout=1;
		else ALUout=0;
		break;
		}
	case 6:
		ALUout=a^b;break;
	case 7:
		ALUout=a/b;break;
	case 8:
		ALUout=a>>b;break;
	case 9:
		ALUout=((long long)a)>>b;break;
	case 10:
		ALUout=a|b;break;
	case 11:
		ALUout=a%b;break;
	case 12:
		ALUout=a&b;break;
	case 13:
		ALUout=PC+b-4;
		break;
	case 14:
		ALUout=b;break;
	case 15:
		ALUout=PC;
	default:;
	}

	//choose reg dst address
	int Reg_Dst;
	if(RegDst){
		Reg_Dst=ID_EX.Rd;
	}
	else
	{
		Reg_Dst=ID_EX.Rt;
	}

	//write EX_MEM_old
	EX_MEM_old.ALU_out=ALUout;
	EX_MEM_old.PC=tempPC;
	EX_MEM_old.Reg_dst=Reg_Dst;
	EX_MEM_old.Zero=Zero;
	EX_MEM_old.Reg_Rt=ID_EX.Reg_Rt;

	EX_MEM_old.Ctrl_M_Branch=ID_EX.Ctrl_M_Branch;
	EX_MEM_old.Ctrl_M_MemWrite=ID_EX.Ctrl_M_MemWrite;
	EX_MEM_old.Ctrl_M_MemRead=ID_EX.Ctrl_M_MemRead;

	EX_MEM_old.Ctrl_WB_RegWrite=ID_EX.Ctrl_WB_RegWrite;
	EX_MEM_old.Ctrl_WB_MemtoReg=ID_EX.Ctrl_WB_MemtoReg;
}

//访问存储器
void MEM()
{
	//read EX_MEM
	REG ALUout=EX_MEM.ALU_out;
	REG RegRt=EX_MEM.Reg_Rt;
	char MemRead=EX_MEM.Ctrl_M_MemRead;
	char MemWrite=EX_MEM.Ctrl_M_MemWrite;

	int tempPC=EX_MEM.PC;
	int Zero=EX_MEM.Zero;
	char Branch=EX_MEM.Ctrl_M_Branch;

	//complete Branch instruction PC change
	if(Branch>4)
		PC=tempPC;
	else if(Branch==1 && Zero==0)
		PC=tempPC;
	else if(Branch==2 && Zero!=1)
		PC=tempPC;
	else if(Branch==3 && Zero<0)
		PC=tempPC;
	else if(Branch==4 && Zero>=0)
		PC=tempPC;

	//read / write memory
	REG MEMout;
	if(MemRead==0){
	}
	else if(MemRead==1){
		MEMout=(char)memory[ALUout>>2];
	}
	else if(MemRead==2){
		MEMout=(short)memory[ALUout>>2];
	}
	else if(MemRead==3){
		MEMout=(int)memory[ALUout>>2];
	}
	else if(MemRead==4){
		MEMout=(long)memory[ALUout>>2];
	}

	if(MemWrite==0){
	}
	else if(MemWrite<4){
		memory[ALUout>>2]=RegRt;
		//printf("ALUout:%llx; RegRt:%lld\n",ALUout,RegRt);
	}
	else if(MemWrite==4){
		memory[ALUout>>2]=(unsigned int)(RegRt>>31);
		memory[(ALUout>>2)+1]=(unsigned int)RegRt;
	}

	//write MEM_WB_old
	MEM_WB_old.Mem_read=MEMout;
	MEM_WB_old.ALU_out=ALUout;
	MEM_WB_old.Reg_dst=EX_MEM.Reg_dst;
		
	MEM_WB_old.Ctrl_WB_RegWrite=EX_MEM.Ctrl_WB_RegWrite;
	MEM_WB_old.Ctrl_WB_MemtoReg=EX_MEM.Ctrl_WB_MemtoReg;
}


//写回
void WB()
{
	//read MEM_WB
	unsigned int Memread=MEM_WB.Mem_read;
	REG ALUout=MEM_WB.ALU_out;
	int Regdst=MEM_WB.Reg_dst;
		
	char RegWrite=MEM_WB.Ctrl_WB_RegWrite;
	char MemtoReg=MEM_WB.Ctrl_WB_MemtoReg;

	//write reg
	REG out;
	if(MemtoReg==0){
		out=ALUout;
	}
	else{
		out=Memread;
	}

	if(RegWrite==1){
		reg[Regdst]=out;
	}
}
