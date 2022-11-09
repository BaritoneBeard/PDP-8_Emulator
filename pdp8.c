// Nicholas Almeder


#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

unsigned short memory[07777];
unsigned short AC=0;
unsigned short PC=0;
unsigned short L=0b0;
unsigned short switchRegister=0;
int halted=0;
int PAGE_LENGTH = 128;
int global_count = 0;

void rotateRight();
void rotateLeft();

struct termios tiosOrig;

void doAnInstruction()
{
	// Instruction format: OOO IZA AAA AAA
	// Opcode - I bit - Z bit - Address

	// fetch instruction from memory
	unsigned short instruction = memory[PC];
	global_count ++;
	//printf("%d PC=%o, L=%o AC=%o, OP=%o ",global_count, PC, L, AC, memory[PC]);	//DEBUG
	// goto next instruction
	unsigned short oldPC = PC;
	PC = (PC + 1) & 07777;		

	// decode
	// parse the opcode
	unsigned short opcode = (instruction >> 9) & 0b111;
	
	// parse the I and Z bits
	unsigned short Imask = (instruction >> 6) & 07;
	unsigned short I = Imask >> 2;
	unsigned short Z = (Imask >> 1) & 1;
	
	// parse the address
	unsigned short addr = instruction & 00177;
	
	// alter address based on I and Z bits
	// If Z is 1, annex number of pages equal to the 5 high order bits from PC
	
	if(Z == 1)
		addr += (((oldPC >> 7) & 0b11111) * PAGE_LENGTH);

	if(I == 1)
	{
		if((addr & 07770) == 010)
		{
			memory[addr] = (memory[addr] + 1) & 07777;
		}
		addr = memory[addr]; 
	}
		
	// Operations based on first octal of instruction
	switch(opcode)
	{
		case 0b000:	// AND M- Memory at location M logically Anded with AC
			if(instruction = 00000)
			{
				PC = (PC + 1) & 07777;
				break;
			}
			AC = AC & memory[addr];
			break;
			
		case 0b001:	// TAD M- Memory at location M added to AC.
			AC = AC + memory[addr];
			if(AC > 07777)
			{
				L = 1-L;
			}

			AC &= 07777;
			break;
			
		case 0b010:	// ISZ M- Increment operand and skip if result zero
			memory[addr] += 1;
			memory[addr] &= 07777;
			// if memory at address becomes 0, skip next instruction
			if(memory[addr] == 0)
			{
				PC += 1;
				PC &= 07777;
			}
			break;
			
		case 0b011:	// DCA M- Deposit AC in memory and clear AC
			memory[addr] = AC;
			AC = 0;
			break;
			
		case 0b100:	// JMS P- Jump to subroutine
			memory[addr] = PC;
			PC = (addr + 1) & 07777;
			break; 
			
		case 0b101:	// JMP P- Jump
			PC = addr;
			break;
			
		// TODO: complete IOT operation, eventually
		case 0b110:	// IOT - input/output transfer
			;
			int device = (instruction >> 3) & 00077;	// middle 6 bits
			int op = instruction & 00007;	// last 3 bits
			
			// Instruction Format: 110 DDD DDD OOO
			// 6, Device bits, operation bits.
			
			switch(device)
			{
				/*case 0:
					// This is where ION and IOF would go, I think.
					break;
					*/
				case 3:
					if(op & 1)	// skip
					{
						PC = (PC + 1) & 07777;
					}
					/*if((op >> 1)&1 == 1)
					{
					}*/
					
					if((op >> 2) == 1)	// get byte
					{
						int c = getchar();
						if (c == 0x1C) // exit program 
						{	
							printf("\r\nQUIT");
					    	}
						AC = c;
					}
					break;
				case 4:
					if ((op & 1) == 1)	// 1: TSF - Teleprinter Skip Flag
				        	PC = (PC + 1) & 07777;
				        if ((op >> 2) == 1)	// 4: TPC - Print Static
				        {
						putchar(AC & 00177); 
						fflush(stdout);
				        }
				        // Place TCF after so AC is not cleared before byte is
				        // sent to teleprinter
				        if (((op >> 1) & 1) == 1) // 2: TCF - Clear Flag
				        	AC = 0;
                       		break;
				default:
					break;
			}
			break;
			
			
		case 0b111:	// OPR - Microcoded Operation
			if(!(instruction & 00400))	// 111 0xxx xxx xxx
			{
				int twice = (instruction >> 1) & 1;
				int CLA = (instruction >> 7) & 1;
				int CLL = (instruction >> 6) & 1;
				int CMA = (instruction >> 5) & 1;
				int CML = (instruction >> 4) & 1;
				int IAC = instruction & 1;
				int RAR = (instruction >> 3) & 1;
				int RAL = (instruction >> 2) & 1;
				int RTR = ((instruction >> 3) & 1) & twice;
				int RTL = ((instruction >> 2) & 1) & twice;
				
				if(CLA)
					AC = 0;
				if(CLL)
					L = 0;
				if(CMA)
					AC ^= 07777;	// XOR to invert all bits
				if(CML)
					L ^= 1;
				if(IAC)
				{
					AC++;
					// If AC == 010000 toggle l
					if(AC > 07777)
						L = 1 - L;
					AC &= 07777;
				}
				if(RTR | RAR)
				{
					switch(twice)
					{
						case 1:
							rotateRight();
							// no break:
							// Fall through into next case to do it twice
						case 0:	
							rotateRight();
							break;
					}
				}
				if(RTL | RAL)
				{
					switch(twice)
					{
						case 1: 
							rotateLeft();
						case 0:
							rotateLeft();
							break;
					}
				} 
							
			}
			else if(instruction & 00400)	// 111 1xx xxx xx0
			{
			
				int skip = 0;
				int s = (instruction >> 3) & 1;
				int SMA = ((instruction >> 6) & 1) & !s;
				int SZA = ((instruction >> 5) & 1) & !s;
				int SNL = ((instruction >> 4) & 1) & !s;
				int SPA = ((instruction >> 6) & 1) & s;
				int SNA = ((instruction >> 5) & 1) & s;
				int SZL = ((instruction >> 4) & 1) & s;
				int CLA = (instruction >> 7) & 1;
				int OSR = (instruction >> 2) & 1;
				int HLT = (instruction >> 1) & 1;
			
				// Skips can be ORed, I don't want to deal with it
				if(SMA)
				{
					if((AC>>11) == 1)
						skip = 1;
				}
				if(SZA)
				{
					if( AC == 0 )
						skip = 1;
				}
				if(SNL)
				{
					if(L != 0)
						skip = 1;
				}
				if(SPA || SNA || SZL)
				{
					int unskip = 0;
					if(SPA)
					{
						if((AC>>11) == 0)
							skip = 1;
						else
							unskip = 1;
					}
					if(SNA)
					{
						if(AC != 0)
							skip = 1;
						else
							unskip = 1;
					}
					if(SZL)
					{
						if(L == 0)
							skip = 1;
						else
							unskip = 1;
					}
					skip = skip - unskip;
				}
				//unconditional skip
				if(((instruction >> 3) & 0b1111) == 0b0001) 
					skip = 1;
				if(CLA)
					AC = 0;	
				if(OSR)
					AC |= switchRegister;
				if(skip == 1)
					PC = (PC + 1) & 07777;
				if(HLT)
					halted = 1;
			}
			break;
			
		default:
			//printf("ERROR: UNKOWN INSTRUCTION!\n");
			break;
	}
}

void rotateRight()
{
	// the old link becomes the most significant bit of AC.
	// do this by adding Lx10e12 to AC, making L bit the first bit of a 13 bit number
	AC += L << 12;
	
	// the least significant bit of AC becomes the new link,
	L = AC & 1;
	
	// restore AC to a 12 bit number now that Link bit has been saved
	AC = AC >> 1;
}

void rotateLeft()
{
	// AC becomes 13 bits
	AC = AC << 1;
	
	// the old link becomes the least significant bit of AC. 
	AC |= L;
	
	// the most significant bit of AC becomes the new link, 
	L = (AC>>12) & 1;
	
	// Restore AC by ANDing the last 12 bits thus ignoring the first one.
	AC &= 07777;	
}

int getOctal(FILE* fp)
{
	// returns 4 octal, from 0000 to 7777
	int value = 0;
	value += ((fgetc(fp) - '0') & 07777)*01000;
	value += ((fgetc(fp) - '0') & 00777)*00100;
	value += ((fgetc(fp) - '0') & 00077)*00010;
	value += ((fgetc(fp) - '0') & 00007);
	return value;
}

void loadProgram(FILE* fp)
{
	int x = 0;
	while(!feof(fp) && x < 07777)
	{
		memory[x] = getOctal(fp);
		x ++;
	}
	
	fclose(fp);
}

void cleanup() 
{
     tcsetattr(0, TCSANOW, &tiosOrig);
}

void printMemory()
{
	for(int printout = 0; printout<07777; printout ++)
	{
		if(printout % 7 == 0)
			printf("\n\r");
		if(printout < 010)
			printf("000");
		else if(printout < 0100 && printout >= 010)
			printf("00");
		else if(printout < 01000 && printout >= 0100)
			printf("0");
		
		printf("%o: %o ", printout, memory[printout]);
	}
	printf("\n\r");
}

int main(int argc, char* argv[])
{
	FILE* fp = fopen(argv[1], "r");
	loadProgram(fp);
	
	if(argv[2]) 	// if third argument exists, set PC to that
		sscanf(argv[2],"%ho",&PC);
	else
		PC = 0200;
	
	// From JohnOH
		struct termios tios = tiosOrig;
		tcgetattr(0, &tiosOrig);
		atexit(cleanup);
		cfmakeraw(&tios);
		tcsetattr(0, TCSANOW, &tios);
	// ----

	while(!halted)
	{
		doAnInstruction();

	}
	
	//printMemory();
	return 0;
}


// Sources of Information
// https://homepage.cs.uiowa.edu/~jones/pdp8/faqs/#instrs
// https://homepage.cs.uiowa.edu/~jones/pdp8/man/mri.html#format
// https://homepage.cs.uiowa.edu/~jones/pdp8/man/micro.html#rar
// etc

// Example of working PDP8 sourcecode from https://github.com/JohnOH/embello/blob/master/explore/1638-pdp8/p8.c
