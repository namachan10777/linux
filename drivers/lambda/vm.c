#include <stdio.h>
#include <stdlib.h>

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

void eval(int *stack, int *ops, int sp, int count) {
    int pc = 0;
    while (pc < count) {
		switch (ops[pc]) {
		case IMM:
			sp += 1;
			stack[sp] = ops[pc+1];
			pc += 2;
			break;

		case ADD:
			stack[sp-1] = stack[sp] + stack[sp-1];
			pc += 1;
			sp -= 1;
			break;

		case SUB:
			stack[sp-1] = stack[sp] - stack[sp-1];
			pc += 1;
			sp -= 1;
			break;

		case MUL:
			stack[sp-1] = stack[sp] * stack[sp-1];
			pc += 1;
			sp -= 1;
			break;

		case DIV:
			stack[sp-1] = stack[sp] / stack[sp-1];
			pc += 1;
			sp -= 1;
			break;

		case MOD:
			stack[sp-1] = stack[sp] % stack[sp-1];
			pc += 1;
			sp -= 1;
			break;

		case PRINT:
			printf("%d\n", stack[sp]);
			sp -= 1;
			pc += 1;
			break;

		case DUP:
			stack[sp+1] = stack[sp];
			sp += 1;
			pc += 1;
			break;
		case IF:
			if (stack[sp]) {
				pc = stack[sp-1];
				sp -= 2;
			}
			else {
				pc += 1;
				sp -= 2;
			}
			break;
		case JMP:
			pc = stack[sp];
			sp -= 1;
			break;
		default:
			printf("invalid op: %d\n", ops[pc]);
			return;
		}
	}
}

int main() {
	int ops[] = {IMM, 10, IMM, 1, IF, IMM, 1, IMM, 12, JMP, IMM, 2, PRINT};
	int *stack = (int*)malloc(sizeof(int) * 1024);
	eval(stack, ops, 0, sizeof(ops)/sizeof(int));
}
