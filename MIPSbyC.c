//
//  Digital System: CourseWork2
//  Created by Xinlong Li / No. 16722011 on 18/6/12.
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define XSTR(x) STR(x)		//can be used for MAX_ARG_LEN in sscanf
#define STR(x) #x

#define MAX_PROG_LEN 250	//maximum number of lines a program can have
#define MAX_LINE_LEN 50		//maximum number of characters a line of code may have
#define MAX_OPCODE   8		//number of opcodes supported (length of opcode_str and opcode_func)
#define MAX_REGISTER 32		//number of registers (size of register_str)
#define MAX_ARG_LEN  20		//used when tokenizing a program line, max size of a token

#define ADDR_TEXT    0x00400000 //where the .text area starts in which the program lives
#define TEXT_POS(a)  ((a==ADDR_TEXT)?(0):(a - ADDR_TEXT)/4) //can be used to access text[]

const char *register_str[] = {	"$zero",
				"$at",
				"$v0", "$v1",
				"$a0", "$a1", "$a2", "$a3",
				"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
				"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
				"$t8", "$t9",
				"$k0", "$k1",
				"$gp",
				"$sp",
				"$fp",
				"$ra"
};

typedef int (*opcode_function)(unsigned int, unsigned int*, char*, char*, char*, char*);

/* Space for the assembler program */
char prog[MAX_PROG_LEN][MAX_LINE_LEN];
int prog_len=0;  //the length of the program

int bytecode_array[MAX_PROG_LEN];  //store the bytecode
char *opcode_array[MAX_PROG_LEN];  //store the opcode

int beq_jump_immediate = 0;  //export the immediate of beq
int bne_jump_immediate = 0;  //export the immediate of bne

unsigned int address = ADDR_TEXT;  //store the address of assembling progress

/* function to create bytecode for instruction nop
 conversion result is passed in bytecode
 function always returns 0 (conversion OK) */
int opcode_nop(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    *bytecode = 0;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "nop";
    address = address + 4;
    return (0);
}

/*add function*/
int opcode_add(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x20;
    
    int i = 0;
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 11;  //add arg1 to bytecode
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 21;  //add arg2 to bytecode
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg3, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;  //add arg3 to bytecode
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "add";
    address = address + 4;
    return (0);
}

/*addi function*/
int opcode_addi(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x20000000;
    
    int i = 0;
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 21;
    
    int arg3_num = atoi(arg3);  //convert arg3 to int type
    if(arg3_num < 0){
        arg3_num = arg3_num & 0x0000ffff;
    }
    *bytecode |= arg3_num;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "addi";
    address = address + 4;
    return (0);
}

/*andi function*/
int opcode_andi(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x30000000;
    
    int i = 0;
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 21;
    
    *bytecode |= atoi(arg3);  //convert arg3 to int type, then add to bytecode
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "andi";
    address = address + 4;
    return (0);
}

/*beq function*/
int opcode_beq(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x10000000;
    
    int i = 0;
    int jump_line = 0;  //Initialize, means the line-number of target to jump
    
    char *target = arg3;  //convert arg3 to string
    strcat(target, ":");  //add ":" to the target, in order to match with target
    
    //find the target, transfer the target to immediate
    for(int line = 0; line < prog_len; line++){
        if (strstr(&prog[line][0], target)){
            jump_line = line;
            break;
        }
    }
    
    int immediate = jump_line - offset - 1;  //The immediate means shift
    beq_jump_immediate = immediate;  //Export immediate to global varibale
    
    if(immediate < 0){
        immediate = immediate & 0x0000ffff;  //deal with negative number
    }
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 21;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    *bytecode |= immediate;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "beq";
    address = address + 4;
    return (0);
}

/*bne function*/
int opcode_bne(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x14000000;
    
    int i = 0;
    
    int jump_line = 0;  //Initialize, means the line-number of target to jump
    char *target = arg3;  //convert arg3 to string
    strcat(target, ":");  //add ":" to the target, in order to match with target
    
    //find the target, transfer the target to immediate
    for(int line = 0; line < prog_len; line++){
        if (strstr(&prog[line][0], target)){
            jump_line = line;
            break;
        }
    }
    
    int immediate = jump_line - offset - 1;
    bne_jump_immediate = immediate; //export the immediate to global variable
    
    if(immediate < 0){
        immediate = immediate & 0x0000ffff;  //deal with negative number
    }
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 21;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    *bytecode |= immediate;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "bne";
    address = address + 4;
    return (0);
}

/*srl function*/
int opcode_srl(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x00000002;
    
    int i = 0;
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 11;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    *bytecode |= atoi(arg3) << 6;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "srl";
    address = address + 4;
    return (0);
}

/*sll function*/
int opcode_sll(unsigned int offset, unsigned int *bytecode, char *opcode, char *arg1, char *arg2, char *arg3 ){
    
    *bytecode = 0x0;
    
    int i = 0;
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg1, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 11;
    
    for (i = 0; i<MAX_REGISTER; i++){
        if (!strcmp(arg2, register_str[i])){
            break;
        }
    }
    *bytecode |= i << 16;
    
    *bytecode |= atoi(arg3) << 6;
    
    printf("0x%08x   0x%08x\n", address, *bytecode);
    bytecode_array[offset] = *bytecode;
    opcode_array[offset] = "sll";
    address = address + 4;
    return (0);
}

const char *opcode_str[] = {"nop", "add", "addi", "andi", "beq", "bne", "srl", "sll"};

opcode_function opcode_func[] = {&opcode_nop, &opcode_add, &opcode_addi, &opcode_andi, &opcode_beq, &opcode_bne, &opcode_srl, &opcode_sll};

/* Elements for running the emulator */
unsigned int registers[MAX_REGISTER] = {0}; // the registers
unsigned int pc = 0; // the program counter
unsigned int text[MAX_PROG_LEN] = {0}; //the text memory with our instructions

/* a function to print the state of the machine */
int print_registers(){
    int i;
    printf("registers:\n");
    
    for(i=0;i<MAX_REGISTER;i++){
        printf(" %d: %d\n", i, registers[i]);
    }
    
    printf(" Program Counter: 0x%08x\n", pc);
    return(0);
}

/* function to execute bytecode */
int exec_bytecode(){
    printf("EXECUTING PROGRAM ...\n");
    pc = ADDR_TEXT; //set program counter to the start of our program
    
    //here goes the code to run the byte code
    int j = 0;  //The loop variable, means the line-number
    
    while(j<prog_len){
        if(strcmp(opcode_array[j],"addi") == 0){
            //L-Type
            int arg1_value = bytecode_array[j] & 0x001f0000;
            arg1_value = arg1_value >> 16;
            int arg2_value = bytecode_array[j] & 0x03e00000;
            arg2_value = arg2_value >> 21;
            int arg3_value = bytecode_array[j] << 16;
            arg3_value = arg3_value >> 16;
            
            registers[arg1_value] = registers[arg2_value] + arg3_value;
        }
        
        else if(strcmp(opcode_array[j],"add") == 0){
            //R-Type
            int arg1_value = bytecode_array[j] & 0x0000f800;
            arg1_value = arg1_value >> 11;
            int arg2_value = bytecode_array[j] & 0x03e00000;
            arg2_value = arg2_value >> 21;
            int arg3_value = bytecode_array[j] & 0x001f0000;
            arg3_value = arg3_value >> 16;
            
            registers[arg1_value] = registers[arg2_value] + registers[arg3_value];
        }
        
        else if(strcmp(opcode_array[j],"andi") == 0){
            //L-Type
            int arg1_value = bytecode_array[j] & 0x001f0000;
            arg1_value = arg1_value >> 16;
            int arg2_value = bytecode_array[j] & 0x03e00000;
            arg2_value = arg2_value >> 21;
            int arg3_value = bytecode_array[j] & 0x0000ffff;
            
            registers[arg1_value] = registers[arg2_value] & arg3_value;
        }
        
        else if(strcmp(opcode_array[j],"srl") == 0){
            //R-Type
            int arg1_value = bytecode_array[j] & 0x0000f800;
            arg1_value = arg1_value >> 11;
            int arg2_value = bytecode_array[j] & 0x001f0000;
            arg2_value = arg2_value >> 16;
            int arg3_value = bytecode_array[j] & 0x000007c0;
            arg3_value = arg3_value >> 6;
            
            registers[arg1_value] = registers[arg2_value] >> arg3_value;
        }
        
        else if(strcmp(opcode_array[j],"sll") == 0){
            //R-Type
            int arg1_value = bytecode_array[j] & 0x0000f800;
            arg1_value = arg1_value >> 11;
            int arg2_value = bytecode_array[j] & 0x001f0000;
            arg2_value = arg2_value >> 16;
            int arg3_value = bytecode_array[j] & 0x000007c0;
            arg3_value = arg3_value >> 6;
            
            registers[arg1_value] = registers[arg2_value] << arg3_value;
        }
        
        else if(strcmp(opcode_array[j],"beq") == 0){
            //J-Type
            int arg1_value = bytecode_array[j] & 0x03e00000;
            arg1_value = arg1_value >> 21;
            int arg2_value = bytecode_array[j] & 0x001f0000;
            arg2_value = arg2_value >> 16;
            
            if(registers[arg1_value] == registers[arg2_value]){
                j = j + beq_jump_immediate;
                pc = pc + 4*beq_jump_immediate;
            }
        }
        
        else if(strcmp(opcode_array[j],"bne") == 0){
            //J-Type
            int arg1_value = bytecode_array[j] & 0x03e00000;
            arg1_value = arg1_value >> 21;
            int arg2_value = bytecode_array[j] & 0x001f0000;
            arg2_value = arg2_value >> 16;
            
            if(registers[arg1_value] != registers[arg2_value]){
                j = j + bne_jump_immediate;
                pc = pc + 4*bne_jump_immediate;
            }
        }
        
        else if(strcmp(opcode_array[j],"nop") == 0){
            //"nop" do nothing
        }
        
        printf("executing   ");
        printf("0x%08x", pc);
        printf("   ");
        printf("0x%08x\n",bytecode_array[j]);
        
        pc = pc + 4;  //shift the pc
        
        j++;//Next Line
    }
    
    print_registers(); // print out the state of registers at the end of execution
    
    printf("... DONE!\n");
    return(0);
}

/* function to create bytecode */
int make_bytecode(){
    unsigned int bytecode; // holds the bytecode for each converted program instruction
    int j = 0; //instruction counter (equivalent to program line)
    
    char label[MAX_ARG_LEN+1];
    char opcode[MAX_ARG_LEN+1];
    char arg1[MAX_ARG_LEN+1];
    char arg2[MAX_ARG_LEN+1];
    char arg3[MAX_ARG_LEN+1];
    
    printf("ASSEMBLING PROGRAM ...\n");
    while(j<prog_len){
        memset(label,0,sizeof(label));
        memset(opcode,0,sizeof(opcode));
        memset(arg1,0,sizeof(arg1));
        memset(arg2,0,sizeof(arg2));
        memset(arg3,0,sizeof(arg3));
        
        bytecode=0;  //Initialize
        
        if (strchr(&prog[j][0], ':')){ //check if the line contains a label
            if (sscanf(&prog[j][0],"%" XSTR(MAX_ARG_LEN) "[^:]: %" XSTR(MAX_ARG_LEN) "s %" XSTR(MAX_ARG_LEN) "s %" XSTR(MAX_ARG_LEN)
                       "s %" XSTR(MAX_ARG_LEN) "s", label, opcode, arg1, arg2, arg3) != 5){ //parse the line with label
                if(strstr(&prog[j][0],"nop")){
                }
                else{
                    printf("parse error line %d\n", j);
                    return(-1);
                }
            }
        }
        
        else {
            if (sscanf(&prog[j][0],"%" XSTR(MAX_ARG_LEN) "s %" XSTR(MAX_ARG_LEN) "s %" XSTR(MAX_ARG_LEN)
                       "s %" XSTR(MAX_ARG_LEN) "s", opcode, arg1, arg2, arg3) != 4){ //parse the line without label
                if(strstr(&prog[j][0],"nop")){
                }
                else{
                    printf("parse error line %d\n", j);
                    return(-1);
                }
            }
        }
        
        //Check wheather include opcodes unavaliable
        int check_opcode_pos = 0;
        for(int check_line = 0; check_line < MAX_OPCODE; check_line++){
            char *opcode_in_array = (char*)malloc(sizeof((opcode_str[check_line])+2)*sizeof(char));
            memset(opcode_in_array,0,sizeof(opcode_str[check_line])+2);
            strcat(opcode_in_array,opcode_str[check_line]);
            strcat(opcode_in_array," ");
            if(!strstr(&prog[j][0],opcode_in_array)){
                check_opcode_pos ++;
            }
            else{
                break;
            }
        }
        if(check_opcode_pos == MAX_OPCODE){
            printf("Error: Include operations which are not avaliable!");
            return(-1);
        }
        
        //Match for different functions to make-bytecode
        int i = 0;
        for (i = 0; i <MAX_OPCODE; i++){
            if(strcmp(opcode, opcode_str[i]) == 0) {
                if ((*opcode_func[i])(j, &bytecode, opcode, arg1, arg2, arg3) < 0) {
                    printf("%d: opcode error (assembly: %s %s %s %s)\n", j, opcode, arg1, arg2, arg3);
                    return(-1);
                    break;
                }
            }
        }
        j++;
    }
    
    printf("... DONE!\n");
    return(0);
}

/* loading the program into memory */
int load_program(){
    int j=0;
    FILE *f;
    
    printf("LOADING PROGRAM ...\n");
    
    f = fopen ("prog.txt", "r");
    while(fgets(&prog[prog_len][0], MAX_LINE_LEN, f) != NULL) {
        prog_len++;
    }
    
    printf("PROGRAM:\n");
    for (j=0;j<prog_len;j++){
        printf("%d: %s",j, &prog[j][0]);
    }
    
    return(0);
}

int main(){
    if (load_program()<0) 	return(-1);        
    if (make_bytecode()<0) 	return(-1); 
    if (exec_bytecode()<0) 	return(-1);
   	return(0);
}
