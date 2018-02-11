#include"Read_Elf.h"

FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//Program headers
unsigned int padr=0;
unsigned int psize=0;
unsigned int pnum=0;

//Section Headers
unsigned int sadr=0;
unsigned int ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;

//用于指示 包含节名称的字符串是第几个节（从零开始计数）
unsigned int Index=0;

//字符串表在文件中地址，其内容包括.symtab和.debug节中的符号表
unsigned int stradr=0;


bool open_file()
{
	elf=fopen("elf","w+");
	return true;
}

void read_elf()
{
	//printf("Start read elf!\n");
	if(!open_file())
		return ;
	
	fprintf(elf,"ELF Header:\n");
	read_Elf_header();

	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();

	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();

	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();

	fclose(elf);
}

void read_Elf_header()
{
	//file should be relocated
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);
	
	fprintf(elf," magic number:  %d\n",*(short*)elf64_hdr.e_ident);

	fprintf(elf," Class:  ELFCLASS32\n");
	
	fprintf(elf," Data:  little-endian\n");
		
	fprintf(elf," Version:  %d\n",*(int*)elf64_hdr.e_version.b);

	fprintf(elf," OS/ABI:  System V ABI\n");
	
	fprintf(elf," ABI Version:   \n");
	
	fprintf(elf," Type:  %d\n",*(short*)elf64_hdr.e_type.b);
	
	fprintf(elf," Machine:   %d\n",*(short*)elf64_hdr.e_machine.b);

	fprintf(elf," Version:  %d\n",*(int*)elf64_hdr.e_version.b);

	fprintf(elf," Entry point address:  0x%lx\n",*(long*)elf64_hdr.e_entry.b);
	entry=*(long*)elf64_hdr.e_entry.b;

	fprintf(elf," Start of program headers:  %ld bytes into  file\n",*(long*)elf64_hdr.e_phoff.b);
	padr=*(long*)elf64_hdr.e_phoff.b;

	fprintf(elf," Start of section headers:  %ld bytes into  file\n",*(long*)elf64_hdr.e_shoff.b);
	sadr=*(long*)elf64_hdr.e_shoff.b;

	fprintf(elf," Flags:  0x%x\n",*(int*)elf64_hdr.e_flags.b);

	fprintf(elf," Size of this header:  %d Bytes\n",*(short*)elf64_hdr.e_ehsize.b);

	fprintf(elf," Size of program headers:  %d Bytes\n",*(short*)elf64_hdr.e_phentsize.b);
	psize=*(short*)elf64_hdr.e_phentsize.b;

	fprintf(elf," Number of program headers:  %d \n",*(short*)elf64_hdr.e_phnum.b);
	pnum=*(short*)elf64_hdr.e_phnum.b;

	fprintf(elf," Size of section headers:  %d Bytes\n",*(short*)elf64_hdr.e_shentsize.b);
	ssize=*(short*)elf64_hdr.e_shentsize.b;

	fprintf(elf," Number of section headers:  %d \n",*(short*)elf64_hdr.e_shnum.b);
	snum=*(short*)elf64_hdr.e_shnum.b;

	fprintf(elf," Section header string table index:  %d \n",*(short*)elf64_hdr.e_shstrndx.b);
	Index=*(short*)elf64_hdr.e_shstrndx.b;
}


char* section_header_string_name(int i){
	FILE *namefile=fopen("../samples/a.out","rb");
    	fseek(namefile,sadr+ssize*(Index),0);
    	Elf64_Shdr elf64_shdr;
    	fread(&elf64_shdr,1,sizeof(elf64_shdr),namefile);
    	long p_offset=*(long*)elf64_shdr.sh_offset.b;
    	long p_size=*(long*)elf64_shdr.sh_size.b;
    	fseek(namefile,p_offset+i,0);
    	char *name=new char[p_size];
    	fread(name,1,sizeof(p_size),namefile);
    	fclose(namefile);
    	return name;
}

void read_elf_sections()
{

	Elf64_Shdr elf64_shdr;

	fseek(file,sadr,0);

	for(int c=0;c<snum;c++)
	{
		fprintf(elf," [%3d]\n",c);
		
		//file should be relocated
		fread(&elf64_shdr,1,sizeof(elf64_shdr),file);

		int header_name_index=*(int*)elf64_shdr.sh_name.b;
		char *header_name=section_header_string_name(header_name_index);
		fprintf(elf," Name:%d-%s; ",header_name_index,header_name);	
		fprintf(elf," Type:%d; ",*(int*)elf64_shdr.sh_type.b);
		fprintf(elf," Address:0x%lx; ",*(long*)elf64_shdr.sh_addr.b);
		fprintf(elf," Offest:0x%lx; \n",*(long*)elf64_shdr.sh_offset.b);
		fprintf(elf," Size:0x%lx; ",*(long*)elf64_shdr.sh_size.b);
		fprintf(elf," Entsize:0x%lx; ",*(long*)elf64_shdr.sh_entsize.b);
		fprintf(elf," Flags:%ld ",*(long*)elf64_shdr.sh_flags.b);
		fprintf(elf," Link:%d ",*(int*)elf64_shdr.sh_link.b);
		fprintf(elf," Info:%d ",*(int*)elf64_shdr.sh_info.b);
		fprintf(elf," Align:%lx \n",*(long*)elf64_shdr.sh_addralign.b);

		if(strcmp(header_name,".data")==0){
			data_adr=*(long*)elf64_shdr.sh_offset.b;
                        data_size=*(long*)elf64_shdr.sh_size.b;
                        data_vadr=*(long*)elf64_shdr.sh_addr.b;
		}
		else if(strcmp(header_name,".bss")==0){
			sdata_adr=*(long*)elf64_shdr.sh_offset.b;
                        sdata_size=*(long*)elf64_shdr.sh_size.b;
                        sdata_vadr=*(long*)elf64_shdr.sh_addr.b;
		}
		else if(strcmp(header_name,".text")==0){
			cadr=*(long*)elf64_shdr.sh_offset.b;
			csize=*(long*)elf64_shdr.sh_size.b;
			vadr=*(long*)elf64_shdr.sh_addr.b;
		}
		else if(strcmp(header_name,".symtab")==0){
                        symadr=*(long*)elf64_shdr.sh_offset.b;
			symsize=*(long*)elf64_shdr.sh_size.b;
			symnum=symsize/ *(long*)elf64_shdr.sh_entsize.b;	
		}
		else if(strcmp(header_name,".strtab")==0){
			stradr=*(long*)elf64_shdr.sh_offset.b;
		}
 	}
}

void read_Phdr()
{
	Elf64_Phdr elf64_phdr;

	fseek(file,padr,0);
	
	for(int c=0;c<pnum;c++)
	{
		fprintf(elf," [%3d]\n",c);
	
		//file should be relocated
		fread(&elf64_phdr,1,sizeof(elf64_phdr),file);

		fprintf(elf," Type:%d; ",*(int*)elf64_phdr.p_type.b);
		
		fprintf(elf," Flags:%x; ",*(int*)elf64_phdr.p_flags.b);
		
		fprintf(elf," Offset:%ld; ",*(long*)elf64_phdr.p_offset.b);
		
		fprintf(elf," VirtAddr:%lx; ",*(long*)elf64_phdr.p_vaddr.b);
		
		fprintf(elf," PhysAddr:%lx; ",*(long*)elf64_phdr.p_paddr.b);

		fprintf(elf," FileSiz:%ld; ",*(long*)elf64_phdr.p_filesz.b);

		fprintf(elf," MemSiz:%ld; ",*(long*)elf64_phdr.p_memsz.b);
		
		fprintf(elf," Align:%ld; ",*(long*)elf64_phdr.p_align.b);
	}
}


char* symbol_table_string_name(int i){
        FILE *namefile=fopen("../samples/a.out","rb");
        fseek(namefile,stradr+i,0);
        char *name=new char[100];
        fread(name,1,100,namefile);
        fclose(namefile);
        return name;
}

void read_symtable()
{
	Elf64_Sym elf64_sym;

	fseek(file,symadr,0);
	

	for(int c=0;c<symnum;c++)
	{
		fprintf(elf," [%3d]   ",c);
		
		//file should be relocated
		fread(&elf64_sym,1,sizeof(elf64_sym),file);

		int name_index=*(int*)elf64_sym.st_name.b;
                char *name=symbol_table_string_name(name_index);
		fprintf(elf," Name:%d-%s; ",name_index,name);
		fprintf(elf," Bind:%d; ",(elf64_sym.st_info)>>4);
		fprintf(elf," Type:%d; ",(elf64_sym.st_info)&0xf);
		fprintf(elf," NDX: %d; ",*(short*)elf64_sym.st_shndx.b);
		fprintf(elf," Size: %lx; ",*(long*)elf64_sym.st_size.b);
		fprintf(elf," Value: %lx;  \n",*(long*)elf64_sym.st_value.b);

		if(strcmp(name,"main")==0){
			madr=*(int*)elf64_sym.st_value.b;
		}
		else if(strcmp(name,"__global_pointer$")==0){
			gp=*(int*)elf64_sym.st_value.b;
		}
	}

}


