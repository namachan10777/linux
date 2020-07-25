#include "json.h"
#include <linux/slab.h>
#include <linux/module.h>

struct List {
	struct JsonValue json;
	struct List *next;
};

struct KeyValueList {
	struct JsonValue json;
	struct String key;
	struct KeyValueList *next;
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
	long long origin = (long long)input;
	int is_true = 1;
	char true_str[] = "true";
	int is_false = 1;
	char false_str[] = "false";
	ParseResult result;
	while(is_white(*input) || count <= 0) {
		++input;
		--count;
	}
	if (count == 0) {
		return gen_error((long long)input);
	}
	if (*input == '"') {
		int i, size=0;
		for (i=1;; ++i, ++size) {
			if (i >= count) return gen_error((long long)input);
			else if (input[i] == '\\' && input[i+1] == '"') {
				if (i+2 >= count) return gen_error((long long)input);
				++i;
			}
			else if (input[i] == '"') {
				char *buf = (char*)kmalloc(size, GFP_KERNEL);
				struct JsonValue value;
				int j, dest;
				for (j=1, dest=0;j<i; ++j, ++dest) {
					if (input[j] == '\\' && input[j+1] == '"') {
						buf[dest] = input[j+1];
						++j;
					}
					else {
						buf[dest] = input[j];
					}
				}
				result.type = SUCCESS;
				value.type = STRING;
				value.string.buf = buf;
				value.string.len = size;
				result.value = value;
				result.used = (long long)input - origin + i + 1;
				return result;
			}
		}
		return gen_error((long long)input);
	}
	if (*input == '-' || is_digit(*input)) {
		int sign = 1;
		int base=0;
		int i=0;
		struct JsonValue value;
		if (count < 1) return gen_error((long long)input);
		if (*input == '-') {
			sign = -1;
			input++;
			count--;
		}
		while (i < count && is_digit(input[i])) {
			base *= 10;
			base += input[i++] - '0';
		}
		if (i == 0) return gen_error((long long)input);
		result.type = SUCCESS;
		value.integer = base * sign;
		value.type = INTEGER;
		result.value = value;
		result.used = i + (long long)input - origin;
		return result;
	}
	if (*input == '[') {
		struct List *list = NULL;
		struct List *l1 = list;
		int cnt = 0;
		int idx = cnt;
		struct JsonValue *arr;
		++input; --count;
		if (count < 0) return gen_error((long long)input);
		while (is_white(*input)) {
			if (count < 0) return gen_error((long long)input);
			++input; --count;
		}
		if (count < 0) return gen_error((long long)input);
		if (*input == ']') {
			result.type = SUCCESS;
			result.used = (long long)input - (long long)origin + 1;
			result.value.type = ARRAY;
			result.value.arrary.len = 0;
			result.value.arrary.arr = NULL;
			return result;
		}

		for (;;) {
			ParseResult result = parse_impl(count, input);
			struct List *next;
			if (result.type == ERROR) {
				result.type = ERROR;
				result.pos = (long long)input;
				return result;
			}
			input += result.used;
			count -= result.used;
			next = kmalloc(sizeof(struct List), GFP_KERNEL);
			next->next = list;
			next->json = result.value;
			list = next;

			if (count < 1) return gen_error((long long)input);
			while (is_white(*input)) {
				if (count <= 1) return gen_error((long long)input);
				++input; --count;
			}

			if (*input == ',') {
				if (count < 1) return gen_error((long long)input);
				++input; --count;
			}
			else if (*input == ']') {
				if (count < 1) return gen_error((long long)input);
				break;
			}
		}
		l1 = list;
		while(l1 != NULL) {
			++cnt;
			l1 = l1->next;
		}
		idx = cnt;
		arr = kmalloc(sizeof(struct JsonValue) * cnt, GFP_KERNEL);
		while(list != NULL) {
			arr[--idx] = list->json;
			list = list->next;
		}
		result.type = SUCCESS;
		result.used = (long long)input - origin + 1;
		result.value.type = ARRAY;
		result.value.arrary.arr = arr;
		result.value.arrary.len = cnt;
		return result;
	}
	if (*input == '{') {
		int cnt = 0;
		struct KeyValueList *l1, *list=NULL;
		int idx;
		struct Pair *pairs;
		++input; --count;
		if (count <= 0) return gen_error((long long)input);
		while (is_white(*input)) {
			if (count < 0) return gen_error((long long)input);
			++input; --count;
		}
		if (count <= 0) return gen_error((long long)input);
		if (*input == '}') {
			result.type = SUCCESS;
			result.used = (long long)input - (long long)origin + 1;
			result.value.type = ARRAY;
			result.value.arrary.len = 0;
			result.value.arrary.arr = NULL;
			return result;
		}

		for (;;) {
			ParseResult result = parse_impl(count, input);
			struct KeyValueList *next;
			struct String key;
			if (result.type == ERROR || result.value.type != STRING) {
				result.type = ERROR;
				result.pos = (long long)input;
				return result;
			}
			key = result.value.string;
			input += result.used;
			count -= result.used;
			while (is_white(*input)) {
				if (count < 0) return gen_error((long long)input);
				++input;
				--count;
			}
			if (*input != ':') return gen_error((long long)input);
			++input;
			--count;
			result = parse_impl(count, input);
			if (result.type == ERROR) {
				result.type = ERROR;
				result.pos = (long long)input;
				return result;
			}
			input += result.used;
			count -= result.used;
			next = kmalloc(sizeof(struct KeyValueList), GFP_KERNEL);
			next->next = list;
			next->json = result.value;
			next->key = key;
			list = next;

			if (count < 1) return gen_error((long long)input);
			while (is_white(*input)) {
				if (count < 1) return gen_error((long long)input);
				++input; --count;
			}

			if (*input == ',') {
				if (count < 1) return gen_error((long long)input);
				++input; --count;
			}
			else if (*input == '}') {
				if (count < 1) return gen_error((long long)input);
				break;
			}
		}
		l1 = list;
		while(l1 != NULL) {
			++cnt;
			l1 = l1->next;
		}
		pairs = kmalloc(sizeof(struct Pair) * cnt, GFP_KERNEL);
		idx = cnt;
		while(list != NULL) {
			pairs[--idx] = (struct Pair){ list->key, list->json };
			list = list->next;
		}
		result.type = SUCCESS;
		result.used = (long long)input - origin + 1;
		result.value.type = OBJECT;
		result.value.pairs.pairs = pairs;
		result.value.arrary.len = cnt;
		return result;
	}
	if (count >= 4) {
		int i;
		for (i = 0; i < 4; ++i) {
			is_true &= true_str[i] == input[i];
		}
	}
	if (count >= 5) {
		int i;
		for (i = 0; i < 5; ++i) {
			is_false &= false_str[i] == input[i];
		}
	}
	if (is_true || is_false) {
		struct JsonValue value;
		result.type = SUCCESS;
		value.type = BOOLEAN;
		value.boolean = is_true;
		result.value = value;
		result.used = (long long)input - origin + is_true ? 4 : 5;
		return result;
	}
	return gen_error((long long)input);
}

ParseResult parse(const char *input, int count) {
	ParseResult result;
	result = parse_impl(count, input);
	if (result.type == ERROR) {
		result.pos -= (long long)input;
	}
	return result;
}

int stringify_impl(char *buf, int buf_size, JSONValue json) {
	if (buf_size < 1) return -1;
	if (json.type == STRING) {
		buf[0] = '"';
		int cursor = 1, i;
		for (i=0; i<json.string.len; ++i) {
			if (json.string.buf[i] == '"') {
				if (cursor+2 >= buf_size) return -1;
				buf[cursor++] = '\\';
				buf[cursor++] = '"';
			}
			else if (json.string.buf[i] == '\\') {
				if (cursor+2 >= buf_size) return -1;
				buf[cursor++] = '\\';
				buf[cursor++] = '\\';
			}
			else {
				if (cursor+1 >= buf_size) return -1;
				buf[cursor++] = json.string.buf[i];
			}
		}
		if (cursor+1 >= buf_size) return -1;
		buf[cursor++] = '\"';
		return cursor;
	}
	else if (json.type == INTEGER) {
		int i=0;
		int base = 1;
		int abs = json.integer > 0 ? json.integer : -json.integer;
		char tmp[1024];
		int j=0, k;
		if (i >= buf_size-1) return -1;
		if (json.integer < 0)
			buf[i++] = '-';
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
		int offset = 1;
		int i;
		if (buf_size < 1) return -1;
		buf[0] = '[';
		for (i=0; i<json.arrary.len; ++i) {
			int s;
			if (buf_size - offset < 1) return -1;
			s = stringify_impl(buf + offset, buf_size-1, json.arrary.arr[i]);
			if (s == -1) return -1;
			offset += s;
			if (i < json.arrary.len-1) {
				if (buf_size - offset < 2) return -1;
				buf[offset++] = ',';
			}
		}
		if (buf_size - offset < 1) return -1;
		buf[offset++] = ']';
		return offset;
	}
	else if (json.type == OBJECT ) {
		int offset = 1;
		int i, j;
		if (buf_size < 1) return -1;
		buf[0] = '{';
		for (i=0; i<json.pairs.len; ++i) {
			int s;
			buf[offset] = '"';
			for (j=0; j<json.pairs.pairs[i].key.len; ++j) {
				buf[++offset] = json.pairs.pairs[i].key.buf[j];
			}
			if (buf_size - offset < 2) return -1;
			buf[++offset] = '"';
			buf[++offset] = ':';
			++offset;
			s = stringify_impl(buf + offset, buf_size-offset, json.pairs.pairs[i].value);
			if (s == -1) return -1;
			offset += s;
			if (i < json.arrary.len-1) {
				if (buf_size - offset < 2) return -1;
				buf[offset++] = ',';
			}
		}
		if (buf_size - offset < 1) return -1;
		buf[offset++] = '}';
		return offset;
	}
	else if (json.type == BOOLEAN) {
		if (json.boolean) {
			int i;
			char src[] = "true";
			if (buf_size < sizeof(src)) return -1;
			for (i = 0; i < sizeof(src); ++i) {
				buf[i] = src[i];
			}
			return sizeof(src)-1;
		}
		else {
			char src[] = "false";
			int i;
			if (buf_size < sizeof(src)) return -1;
			for (i = 0; i < sizeof(src); ++i) {
				buf[i] = src[i];
			}
			return sizeof(src)-1;
		}
	}
	else {
		return 0;
	}
}

int stringify(char *buf, int buf_size, JSONValue json) {
	int len = stringify_impl(buf, buf_size-1, json);
	if (len >= 0) {
		buf[len] = '\0';
		return len+1;
	}
	return -1;
}

MODULE_LICENSE("GPL");
