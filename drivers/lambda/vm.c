#include "vm.h"

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
			return;
		}
	}
}
