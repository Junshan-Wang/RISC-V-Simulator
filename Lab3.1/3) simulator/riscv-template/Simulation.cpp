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

extern unsigned int data_adr[4];
extern unsigned int data_vadr[4];
extern unsigned int data_size[4];

//单步模式
int debug=0;

//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;

//模拟器周期数
long long cycle_num=0;

//冒险次数
long long data_hazards=0;
long long control_hazards=0;


bool controlHazards=false;
//加载代码段
//初始化PC
void load_memory()
{
	//printf("code address: %x,size: %x\n",vadr>>2,csize);
	//printf("file offset: %x\n",cadr);

	fseek(file,cadr,SEEK_SET);
	//fread(&memory[vadr>>2],1,csize,file);
	char *content=new char[csize];
	fread(content,1,csize,file);
	memory.WriteMemory(vadr,csize,content);
	delete content;

	vadr=vadr>>2;
	csize=csize>>2;

	for(int i=0;i<3;++i){
		//printf("data vaddr: %x\n",data_vadr[i]);
		fseek(file,data_adr[i],SEEK_SET);
		//fread(&memory[data_vadr[i]>>2],1,data_size[i],file);
		char *content=new char[data_size[i]];
		fread(content,1,data_size[i],file);
		memory.WriteMemory(data_vadr[i],data_size[i],content);
		delete content;

		data_vadr[i]=data_vadr[i]>>2;
		data_size[i]=data_size[i]>>2;
	}

	fclose(file);
}

int main(int argc,char*argv[])
{
	debug=argv[1][0]-48;
	char mode=argv[2][0];
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

	if(mode=='0'){
		cout<<"Simple"<<endl;
		simulate_simple();
		//simulate_multiple();
	}
	else if(mode=='1'){
		cout<<"Multiple"<<endl;
		simulate_multiple();
	}
	else{
		cout<<"Pipeline"<<endl;
		simulate_pipeline();
	}

	cycle_num+=memory.AccessCycle();

	cout<<"simulate over!"<<endl;
	cout<<"Display Status?(y/n)";
	char *c=new char[100];
	cin>>c;
	if(c[0]=='y')
		PrintStatus();
	return 0;
}

//单周期处理器
void simulate_simple()
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

                if(exit_flag==1)
                        break;

                reg[0]=0;//一直为零

                if(debug){
                        PrintStatus();
                }

		cycle_num++;
                inst_num++;
        }
}

//多周期处理器
void simulate_multiple()
{
	//结束PC的设置
        int end=(int)endPC/4-1;
        while(PC!=0)
        {
                //运行
                IF();IF_ID=IF_ID_old;cycle_num+=1;
                ID();ID_EX=ID_EX_old;cycle_num+=1;
                EX();EX_MEM=EX_MEM_old;cycle_num+=1;
                MEM();MEM_WB=MEM_WB_old;cycle_num+=1;
                WB();cycle_num+=1;

                if(exit_flag==1)
                        break;

                reg[0]=0;//一直为零

                if(debug){
                        PrintStatus();
                }

                inst_num++;
        }
}

//流水线处理器
void simulate_pipeline()
{
	//结束PC的设置
	int end=(int)endPC/4-1;
	InitializePipeline();
	IF();IF_ID=IF_ID_old;	
	while(PC!=0)
	{
		//cout<<"run"<<endl;
		//运行
		if(!DataHazards()){
			IF();//IF_ID=IF_ID_old;
			ID();//ID_EX=ID_EX_old;
			inst_num+=1;
		}
		else{
			nop();
			data_hazards+=1;
			//cout<<"Stop!"<<endl;
		}
		EX();//EX_MEM=EX_MEM_old;
		MEM();//MEM_WB=MEM_WB_old;
		WB();

		if(controlHazards){
			cover();
			controlHazards=false;
			inst_num-=1;
			control_hazards+=1;
		}

		//更新中间寄存器
		IF_ID=IF_ID_old;
		ID_EX=ID_EX_old;
		EX_MEM=EX_MEM_old;
		MEM_WB=MEM_WB_old;

        	if(exit_flag==1)
            		break;

        	reg[0]=0;//一直为零

        	if(debug){
        		PrintStatus();
        	}
		
		cycle_num+=1;
	}
	WB();
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
	//cout<<data_vadr[0]<<" "<<data_size<<endl;
	//printf("data_vadr[0]:%x, data_size:%x\n",data_vadr[0]<<2,data_size[0]<<2);
	//for(int i=data_vadr[0];i<data_vadr[0]+50;i+=1){
	
	//for(int i=(0x11010>>2);i<=(0x11010>>2)+50;i++){ /*add.c, mul-div.c, simple-fuction.c*/
	//for(int i=(0x117b0>>2);i<=(0x117b0>>2)+50;i++){ /*qsort.c*/
	for(int i=(0x11770>>2);i<=(0x11770>>2)+50;i++){ /*n!.c*/
	//for(int i=(0x11770>>2);i<=(0x11770>>2)+50;i++){
		int ans=memory.ReadMemory(i<<2,4);
		if(i<MAX) printf("%x: %x; ",i<<2,ans);
		//if(i<MAX) printf("%x: %x; ",i<<2,memory[i]);
		else { cout<<"out of memory"; break;}
	}
	//printf("%x: %x; ",0x11770,memory[0x11770>>2]);
	cout<<endl;
	
	/*cout<<"------Memory: gp------"<<endl;
	//cout<<gp<<endl;
	printf("gp:%llx\n",gp);
	for(int i=(gp>>2);i<(gp>>2)+10;i+=1){
		if(i<MAX) cout<<memory[i]<<" ";
		else { cout<<"out of memory"; break;}
	}
	cout<<endl;*/

	/*cout<<"------Stack------"<<endl;
	printf("stack addr:%llx\n",reg[2]);
        for(int i=(reg[2]>>2);i<(reg[2]>>2)+100;i+=1){
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
	
	cout<<">>>>>>> Instruction Number: "<<inst_num<<endl;	
	cout<<">>>>>>> Cycle Number: "<<cycle_num<<endl;
	cout<<">>>>>>> CPI: "<<((double)cycle_num)/inst_num<<endl;
	if(data_hazards>0 && control_hazards>0){
		cout<<">>>>>>> Data Hazards Number: "<<data_hazards<<endl;
		cout<<">>>>>>> Control Hazards Number: "<<control_hazards<<endl;
		cout<<">>>>>>> All Hazards Number: "<<data_hazards+control_hazards<<endl;
	}	
	cin.get();
	cout<<endl<<"========== NEXT =========="<<endl;
}


//初始化流水线
void InitializePipeline()
{
	ID_EX.Ctrl_M_MemWrite=0;
	ID_EX.Ctrl_WB_RegWrite=0;

	EX_MEM.Ctrl_M_MemWrite=0;
	EX_MEM.Ctrl_WB_RegWrite=0;

	MEM_WB.Ctrl_WB_RegWrite=0;
}


//数据冒险
bool DataHazards()
{
	unsigned int inst=IF_ID.inst;
	char RegDst=ID_EX.Ctrl_EX_RegDst;
	int rs,rt,rw_ex,rw_mem,rw_wb,OP;
	char RegW_ex,RegW_mem,RegW_wb;
	OP=getbit(inst,0,7);
	if(OP==OP_AUIPC || OP==OP_LUI || OP==OP_JAL)
		return false;
	rs=getbit(inst,15,20);
	rt=getbit(inst,20,25);
	//cout<<"rs:"<<rs<<"; rt:"<<rt<<endl;
        if(RegDst)
                rw_ex=ID_EX.Rd;
        else
                rw_ex=ID_EX.Rt;
	rw_mem=EX_MEM.Reg_dst;
	rw_wb=MEM_WB.Reg_dst;
	RegW_ex=ID_EX.Ctrl_WB_RegWrite;
	RegW_mem=EX_MEM.Ctrl_WB_RegWrite;
	RegW_wb=MEM_WB.Ctrl_WB_RegWrite;	
	if((rs==rw_ex || rt==rw_ex) && RegW_ex){
		if(debug) cout<<"Stop ex: "<<rs<<" "<<rt<<" "<<rw_ex<<" "<<(int)RegW_ex<<endl;
		return true;	
	}
	if((rs==rw_mem || rt==rw_mem) && RegW_mem){
		if(debug) cout<<"Stop mem: "<<rs<<" "<<rt<<" "<<rw_mem<<" "<<(int)RegW_mem<<endl;
		return true;
	}
	if((rs==rw_wb || rt==rw_wb) && RegW_wb){
		if(debug) cout<<"Stop wb: "<<rs<<" "<<rt<<" "<<rw_wb<<" "<<(int)RegW_wb<<endl;
		return true;
	}
	return false;
}

//插入NOP
void nop()
{
	ID_EX_old.Ctrl_M_Branch=0;
	ID_EX_old.Ctrl_M_MemWrite=0;
	ID_EX_old.Ctrl_WB_RegWrite=0;	
}

//碾压错误指令
void cover()
{
	if(debug) cout<<"Cover"<<endl;
	IF_ID_old.inst=0;

	ID_EX_old.Ctrl_M_Branch=0;	
	ID_EX_old.Ctrl_M_MemWrite=0;
        ID_EX_old.Ctrl_WB_RegWrite=0;
}


//取指令
void IF()
{
	//write IF_ID_old
	IF_ID_old.inst=memory.ReadMemory(PC,4);
	//IF_ID_old.inst=memory[PC>>2];
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
	if(inst==0){
		MemWrite=0;
		RegWrite=0;
		if(debug) printf("nop\n");
	}	
	else if(OP==OP_R || OP==OP_RW)
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
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
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
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 2:
			switch(fuc7){
			case 0:
				ALUOp=5;
				if(debug) printf("slt %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				if(debug) printf("Bad operator!  PC: %x\n", PC);break;
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
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 5:
			switch(fuc7){
			case 0:
				ALUOp=8;
				if(debug) printf("xrl %d,%d,%d\n",rd,rs,rt);
				break;
			case 2:
				ALUOp=9;
				if(debug) printf("sra %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
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
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
			}break;
		case 7:
			switch(fuc7){
			case 0:
				ALUOp=12;
				if(debug) printf("and %d,%d,%d\n",rd,rs,rt);
				break;
			default:
				if(debug) printf("Bad operator! PC: %x\n", PC);break;
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
	else if(OP==OP_I || OP==OP_IW)
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
        	if(debug) printf("Bad operator! PC: %x\n", PC);
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
        	if(debug) printf("Bad operator! PC: %x\n", PC);
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
        	if(debug) printf("Bad operator! PC: %x\n", PC);
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
           	if(debug) printf("Bad operator! PC: %x\n", PC);
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
    		if(debug) printf("Bad operator! PC: %x\n", PC);
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
    	EXTop=32;
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
	if(debug) printf("Bad operator! PC: %x\n", PC);
    	}

	//write ID_EX_old
	ID_EX_old.Rd=rd;
	ID_EX_old.Rt=rt;
	ID_EX_old.PC=IF_ID.PC-4;
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
		if(debug) printf("Bad ALUSrc!\n");
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
		ALUout=ID_EX.PC+b;
		break;
	case 14:
		ALUout=b;break;
	case 15:
		ALUout=ID_EX.PC+4;
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


	//complete Branch instruction PC change
        if(Branch>4){
                PC=tempPC;
		controlHazards=true;
	}
        else if(Branch==1 && Zero==0){
                PC=tempPC;
		controlHazards=true;
	}
        else if(Branch==2 && Zero!=0){
                PC=tempPC;
		controlHazards=true;
	}
        else if(Branch==3 && Zero<0){
                PC=tempPC;
		controlHazards=true;
	}
        else if(Branch==4 && Zero>=0){
                PC=tempPC;
		controlHazards=true;
	}

	//write EX_MEM_old
	EX_MEM_old.ALU_out=ALUout;
	EX_MEM_old.PC=PC;
	EX_MEM_old.Reg_dst=Reg_Dst;
	EX_MEM_old.Zero=Zero;
	EX_MEM_old.Reg_Rt=ID_EX.Reg_Rt;

	EX_MEM_old.Ctrl_M_Branch=ID_EX.Ctrl_M_Branch;
	EX_MEM_old.Ctrl_M_MemWrite=ID_EX.Ctrl_M_MemWrite;
	EX_MEM_old.Ctrl_M_MemRead=ID_EX.Ctrl_M_MemRead;

	EX_MEM_old.Ctrl_WB_RegWrite=ID_EX.Ctrl_WB_RegWrite;
	EX_MEM_old.Ctrl_WB_MemtoReg=ID_EX.Ctrl_WB_MemtoReg;

	if(ALUOp==1){
		long long a_32=(int)a;
		long long b_32=(int)b;
		if (a_32==a && b_32==b)
			cycle_num+=0;
		else
			cycle_num+=1;
	}
	else if(ALUOp==7 || ALUOp==11)
		cycle_num+=39;
	else
		cycle_num+=0;
}

//访问存储器
void MEM()
{
	//read EX_MEM
	REG ALUout=EX_MEM.ALU_out;
	REG RegRt=EX_MEM.Reg_Rt;
	char MemRead=EX_MEM.Ctrl_M_MemRead;
	char MemWrite=EX_MEM.Ctrl_M_MemWrite;

	//int tempPC=EX_MEM.PC;
	//int Zero=EX_MEM.Zero;
	//char Branch=EX_MEM.Ctrl_M_Branch;

	//complete Branch instruction PC change
	/*if(Branch>4)
		PC=tempPC;
	else if(Branch==1 && Zero==0)
		PC=tempPC;
	else if(Branch==2 && Zero!=0)
		PC=tempPC;
	else if(Branch==3 && Zero<0)
		PC=tempPC;
	else if(Branch==4 && Zero>=0)
		PC=tempPC;
	*/

	//read / write memory
	/*REG MEMout;
	if(MemRead==0){
	}
	else if(MemRead==1){
		MEMout=(short)memory[ALUout>>2];
	}
	else if(MemRead==2){
		MEMout=(short)memory[ALUout>>2];
	}
	else if(MemRead==3){
		MEMout=(int)memory[ALUout>>2];
		//printf("%llx, %x, %llx\n",ALUout>>2, memory[ALUout>>2],MEMout);
		//cin.get();
	}
	else if(MemRead==4){
		MEMout=(memory[ALUout>>2]<<31)+memory[(ALUout>>2)+1];
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
	}*/

	REG MEMout;
	if(MemRead>0){
		MEMout=memory.ReadMemory(ALUout,MemRead);
	}
	if(MemWrite>0){
		memory.WriteMemory(ALUout,MemWrite,RegRt);
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
        	out=(int)Memread;
    	}

    	if(RegWrite==1){
        	reg[Regdst]=out;
    	}
}
