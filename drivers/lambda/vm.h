#ifndef __VM_H__
#define __VM_H__

enum OPCODE {
	IMM = 0x40,
	ADD = 0x41,
	SUB = 0x42,
	MUL = 0x43,
	DIV = 0x44,
	MOD = 0x45,
	DUP = 0x46,
	PRINT = 0x47,
	IF = 0x48,
	JMP = 0x49
};

void eval(int *stack, int *ops, int sp, int count);

#endif
