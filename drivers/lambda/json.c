#include "json.h"
#include <stdlib.h>
#include <stdio.h>

ParseResult gen_error(int pos) {
	ParseResult result;
	result.type = ERROR;
	result.pos = pos;
	return result;
}

int is_white(char c) {
	return c != ' ' && c != '\t' && c != '\n' && c != '\r';
}

int is_digit(char c) {
	return c >= '0' && c <= '9';
}

ParseResult parse_impl(int begin, int end, const char *input) {
	ParseResult result;
	while(input[begin] == is_white(input[begin])) ++begin;
	if (begin >= end) {
		return gen_error(begin);
	}
	if (input[begin] == '\"') {
		int i;
		for(i=begin+1;;++i) {
			if (i >= end) {
				return gen_error(begin);
			}
			else if (input[i] == '\\' && input[i+1] == '\"') {
				++i;
			}
			else if (input[i] == '\"') {
				int j;
				for (j=i+1;j<end;++j) {
					if (!is_white(input[begin]))
						return gen_error(j);
				}
				char *buf = (char*)malloc(sizeof(char) * (i-begin-1));
				for (j=0; j<(i-begin); ++j) {
					buf[j] = input[begin+j];
				}
				result.type = SUCCESS;
				struct JsonValue value;
				value.type = STRING;
				value.string.buf = buf;
				value.string.len = (i-begin-1);
				result.value = value;
				return result;
			}
		}
	}
	else if (input[begin] == '-' || is_digit(input[begin])) {
		int sign = 1;
		int base=0;
		if (input[begin] == '-') {
			sign = -1;
			begin++;
		}
		int i=begin, j;
		while (i < end && is_digit(input[i])) {
			base *= 10;
			base += input[i++] - '0';
		}
		for (j=i+1;j<end;++j) {
			if (!is_white(input[begin]))
				return gen_error(j);
		}
		result.type = SUCCESS;
		struct JsonValue value;
		value.integer = base * sign;
		value.type = INTEGER;
		result.value = value;
		return result;
	}
	return gen_error(begin);
}

ParseResult parse(const char *input) {
	int len=0;
	while(input[len] != '\0') ++len;
	return parse_impl(0, len, input);
}

int stringify_impl(char *buf, int buf_size, JSONValue json) {
	if (json.type == STRING) {
		if (buf_size < json.string.len + 3) return -1;
		buf[0] = '\"';
		int i;
		for (i=0;i<json.string.len;++i) {
			buf[i] = json.string.buf[i+1];
		}
		buf[json.string.len+1] = '\"';
		return json.string.len+2;
	}
	else if (json.type == INTEGER) {
		int i=0;
		int base = 1;
		int abs = json.integer > 0 ? json.integer : -json.integer;
		if (i >= buf_size-1) return -1;
		if (json.integer < 0)
			buf[i++] = '-';
		char tmp[1024];
		int j=0, k;
		while(abs / base > 0) {
			tmp[j++] = (abs / base) % 10 + '0';
			if (j+i >= buf_size) return -1;
			base *= 10;
		}
		for(k=0;k<j;++k) {
			buf[i+k] = tmp[j-k-1];
		}
		return j+i;
	}
	else {
		return 0;
	}
}

void stringify(char *buf, int buf_size, JSONValue json) {
	int len = stringify_impl(buf, buf_size-1, json);
	if (len >= 0) {
		buf[len] = '\0';
	}
}
