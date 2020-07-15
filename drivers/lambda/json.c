#include "json.h"
#include <stdlib.h>
#include <stdio.h>

struct List {
	struct JsonValue json;
	struct List *next;
};

ParseResult gen_error(int pos) {
	ParseResult result;
	result.type = ERROR;
	result.pos = pos;
	return result;
}

int is_white(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int is_digit(char c) {
	return c >= '0' && c <= '9';
}

ParseResult parse_impl(int count, const char *input) {
	int origin = (int)input;
	ParseResult result;
	while(is_white(*input) || count <= 0) {
		++input;
		--count;
	}
	if (count == 0) {
		return gen_error((int)input);
	}
	if (*input == '\"') {
		int i;
		for(i=1; i < count;++i) {
			if (input[i] == '\\' && input[i+1] == '\"') {
				++i;
			}
			else if (input[i] == '\"') {
				int j;
				char *buf = (char*)malloc(i);
				for (j=0; j<i; ++j) {
					buf[j] = input[j];
				}
				result.type = SUCCESS;
				struct JsonValue value;
				value.type = STRING;
				value.string.buf = buf;
				value.string.len = (i-1);
				result.value = value;
				result.used = (int)input - origin + i;
				return result;
			}
		}
		return gen_error((int)input);
	}
	if (*input == '-' || is_digit(*input)) {
		int sign = 1;
		int base=0;
		if (count < 1) return gen_error((int)input);
		if (*input == '-') {
			sign = -1;
			input++;
			count--;
		}
		int i=0, j;
		while (i < count && is_digit(input[i])) {
			base *= 10;
			base += input[i++] - '0';
		}
		result.type = SUCCESS;
		struct JsonValue value;
		value.integer = base * sign;
		value.type = INTEGER;
		result.value = value;
		result.used = i + (int)input - origin;
		return result;
	}
	if (*input == '[') {
		++input; --count;
		if (count <= 0) gen_error((int)input);
		while (is_white(*input)) {
			if (count < 0) gen_error((int)input);
			++input; --count;
		}
		if (count <= 0) gen_error((int)input);
		if (*input == ']') {
			result.type = SUCCESS;
			result.used = (int)input - (int)origin + 1;
			result.value.type = ARRAY;
			result.value.arrary.len = 0;
			result.value.arrary.arr = NULL;
			return result;
		}
		int len = 0;
		struct List *list = NULL;

		for (;;) {
			ParseResult result = parse_impl(count, input);
			if (result.type == ERROR) {
				result.type = ERROR;
				result.pos = (int)input;
				return result;
			}
			input += result.used;
			count -= result.used;
			struct List *next = malloc(sizeof(struct List));
			next->next = list;
			next->json = result.value;
			list = next;

			if (count <= 1) gen_error((int)input);
			while (is_white(*input)) {
				if (count <= 1) gen_error((int)input);
				++input; --count;
			}

			if (*input == ',') {
				if (count <= 1) gen_error((int)input);
				++input; --count;
			}
			else if (*input == ']') {
				if (count <= 1) gen_error((int)input);
				break;
			}
		}
		struct List *l1 = list;
		int cnt = 0;
		while(l1 != NULL) {
			++cnt;
			l1 = l1->next;
		}
		struct JsonValue *arr = malloc(sizeof(struct JsonValue) * cnt);
		int idx = cnt;
		while(list != NULL) {
			arr[--idx] = list->json;
			list = list->next;
		}
		result.type = SUCCESS;
		result.used = (int)input - origin + 1;
		result.value.type = ARRAY;
		result.value.arrary.arr = arr;
		result.value.arrary.len = cnt;
		return result;
	}
	int is_true = 1;
	char true_str[] = "true";
	if (count >= 4) {
		int i;
		for (i = 0; i < 4; ++i) {
			is_true &= true_str[i] == input[i];
		}
	}
	int is_false = 1;
	char false_str[] = "false";
	if (count >= 5) {
		int i;
		for (i = 0; i < 5; ++i) {
			is_true &= false_str[i] == input[i];
		}
	}
	if (is_true || is_false) {
		result.type = SUCCESS;
		struct JsonValue value;
		value.type = BOOLEAN;
		value.boolean = is_true;
		result.value = value;
		result.used = (int)input - origin + is_true ? 4 : 5;
		return result;
	}
	return gen_error((int)input);
}

ParseResult parse(const char *input) {
	int len=0;
	while(input[len] != '\0') ++len;
	ParseResult result = parse_impl(len, input);
	if (result.type == ERROR) {
		result.pos -= (int)input;
	}
	return result;
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
	else if (json.type == ARRAY) {
		if (buf_size < 1) return -1;
		buf[0] = '[';
		int offset = 1;
		for (int i=0; i<json.arrary.len; ++i) {
			if (buf_size - offset < 1) return -1;;
			int s = stringify_impl(buf + offset, buf_size-1, json.arrary.arr[i]);
			if (s == -1) return -1;
			offset += s;
			if (i < json.arrary.len-1) {
				if (buf_size - offset < 3) return -1;;
				buf[offset++] = ',';
				buf[offset++] = ' ';
			}
		}
		if (buf_size - offset < 1) return -1;
		buf[offset++] = ']';
		return offset;
	}
	else if (json.type == BOOLEAN) {
		if (json.boolean) {
			char src[] = "true";
			if (buf_size < sizeof(src)) return -1;
			int i;
			for (i = 0; i < sizeof(src); ++i) {
				buf[i] = src[i];
			}
			return sizeof(src);
		}
		else {
			char src[] = "false";
			if (buf_size < sizeof(src)) return -1;
			int i;
			for (i = 0; i < sizeof(src); ++i) {
				buf[i] = src[i];
			}
			return sizeof(src);
		}
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
