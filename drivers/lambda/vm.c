#include "vm.h"

enum result eval(struct value *stack, int *ops, int sp, int count) {
    int pc = 0, fp=0;
    while (pc < count) {
		switch (ops[pc]) {
		case IMM:
			sp += 1;
			struct value val;
			val.type = INT;
			val.inner.integer = ops[pc+1];
			stack[sp] = val;
			pc += 2;
			break;

		case ADD:
			if (stack[sp].type != INT || stack[sp-1].type != INT)
				return TYPE_ERROR;
			stack[sp-1].type = INT;
			stack[sp-1].inner.integer = stack[sp].inner.integer + stack[sp-1].inner.integer;
			pc += 1;
			sp -= 1;
			break;

		case SUB:
			if (stack[sp].type != INT || stack[sp-1].type != INT)
				return TYPE_ERROR;
			stack[sp-1].type = INT;
			stack[sp-1].inner.integer = stack[sp].inner.integer - stack[sp-1].inner.integer;
			pc += 1;
			sp -= 1;
			break;

		case MUL:
			if (stack[sp].type != INT || stack[sp-1].type != INT)
				return TYPE_ERROR;
			stack[sp-1].type = INT;
			stack[sp-1].inner.integer = stack[sp].inner.integer * stack[sp-1].inner.integer;
			pc += 1;
			sp -= 1;
			break;

		case DIV:
			if (stack[sp].type != INT || stack[sp-1].type != INT)
				return TYPE_ERROR;
			stack[sp-1].type = INT;
			stack[sp-1].inner.integer = stack[sp].inner.integer / stack[sp-1].inner.integer;
			pc += 1;
			sp -= 1;
			break;

		case MOD:
			if (stack[sp].type != INT || stack[sp-1].type != INT)
				return TYPE_ERROR;
			stack[sp-1].type = INT;
			stack[sp-1].inner.integer = stack[sp].inner.integer % stack[sp-1].inner.integer;
			pc += 1;
			sp -= 1;
			break;

		case DUP:
			stack[sp+1] = stack[sp];
			sp += 1;
			pc += 1;
			break;
		case IF:
			if (stack[sp].type != BOOL || stack[sp-1].type != INT) return TYPE_ERROR;
			if (stack[sp].inner.boolean) {
				pc = stack[sp-1].inner.integer;
				sp -= 2;
			}
			else {
				pc += 1;
				sp -= 2;
			}
			break;
		case JMP:
			if (stack[sp].type != INT) return TYPE_ERROR;
			pc = stack[sp].inner.integer;
			sp -= 1;
			break;
		case CALL: {
			if (stack[sp].type != INT || stack[sp-1].type != INT || stack[sp-2].type != INT) return TYPE_ERROR;
			int ret_n = stack[sp--].inner.integer;
			int arg_n = stack[sp--].inner.integer;
			int to    = stack[sp--].inner.integer;
			for (int ptr=0; ptr < arg_n; ++ptr) {
				stack[sp-arg_n+ret_n+2+ptr] = stack[ptr];
			}
			stack[sp-arg_n+ret_n+1].inner.integer = fp;
			stack[sp-arg_n+ret_n+2].inner.integer = pc;
			fp = sp-arg_n+ret_n+2;
			sp = fp + arg_n;
			pc = to;
			break;
		}
		case RET: {
			sp = fp-2;
			pc = stack[fp].inner.integer;
			fp = stack[fp-1].inner.integer;
			break;
		}
		default:
			return UNKNOWN_OPCODE;
		}
	}
	return SUCC;
}
