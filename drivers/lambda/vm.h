#ifndef __VM_H__
#define __VM_H__

enum result {
	TYPE_ERROR,
	SUCC,
	UNKNOWN_OPCODE,
};

enum type {
	INT,
	STRING,
	BOOL
};

struct value {
	enum type type;
	union {
		int boolean;
		int integer;
		char *str;
	} inner;
};

enum OPCODE {
	IMM = 0x40,
	ADD = 0x41,
	SUB = 0x42,
	MUL = 0x43,
	DIV = 0x44,
	MOD = 0x45,
	DUP = 0x46,
	IF = 0x48,
	JMP = 0x49,
	CALL = 0x60,
	RET = 0x61,
};

enum result eval(struct value *stack, int *ops, int sp, int count);

#endif
