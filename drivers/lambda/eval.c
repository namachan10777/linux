#include "json.h"

/*#include <linux/module.h>*/
#include "stub.h"
#include <stdio.h>
#include <stdlib.h>

#define STRING(name, inner) struct String name = { inner, sizeof(inner)-1};

/* TODO GC */

struct Path {
	int len;
	struct PathElem *path;
};

struct PathElem {
	int is_ref;
	union {
		struct Path path;
		char *name;
	};
};

struct List {
	struct List *next;
	struct PathElem e;
};

struct JsonValue *empty_object() {
	struct JsonValue *json = (struct JsonValue*)malloc(sizeof(struct JsonValue));
	json->type = OBJECT;
	json->pairs.len = 0;
	json->pairs.mem_len = 0;
	return json;
}

int parse_path(struct Path *path_buf, char *buf, int count) {
	int len=0, idx=0, begin=0;
	int j;
	struct List *list = NULL;
	while (idx < count) {
		if (buf[idx] == '.') {
			char *name = (char*)kmalloc(idx-begin+1, GFP_KERNEL);
			for (j=0;j<idx-begin; ++j) {
				name[j] = buf[begin+j];
			}
			name[idx-begin] = '\0';
			struct List *new = (struct List*)kmalloc(sizeof(struct List), GFP_KERNEL);
			new->e.is_ref = 0;
			new->e.name = name;
			new->next = list;
			list = new;
			begin = idx+1;
			++len;
		}
		else if (buf[idx] == '[') {
			int end = idx+1;
			int paren_count = 0;
			int succ=0;
			while (end<count) {
				if (buf[end] == ']') {
					if (paren_count == 0) {
						succ=1;
						break;
					}
					else {
						--paren_count;
					}
				}
				else if (buf[end] == '[') {
					++paren_count;
				}
				++end;
			}
			if (!succ) return 0;
			struct Path sub;
			if (!parse_path(&sub, buf+idx+1, end-idx-1))
				return 0;
			struct List *new = (struct List*)kmalloc(sizeof(struct List), GFP_KERNEL);
			new->e.is_ref = 0;
			char *name = (char*)kmalloc(idx-begin+1, GFP_KERNEL);
			for (j=0;j<idx-begin; ++j) {
				name[j] = buf[begin+j];
			}
			new->e.name = name;
			new->next = list;
			list = new;
			new = (struct List*)kmalloc(sizeof(struct List), GFP_KERNEL);
			new->e.is_ref = 1;
			new->e.path = sub;
			new->next = list;
			list = new;
			begin = end;
			idx = end;
			len+=2;
		}
		++idx;
	}
	if (count-begin > 1) {
		char *name = (char*)kmalloc(count-begin+1, GFP_KERNEL);
		for (j=0;j<count-begin; ++j) {
			name[j] = buf[begin+j];
		}
		name[idx-begin] = '\0';
		struct List *new = (struct List*)kmalloc(sizeof(struct List), GFP_KERNEL);
		new->e.is_ref = 0;
		new->e.name = name;
		new->next = list;
		list = new;
		++len;
	}
	struct PathElem *elems = (struct PathElem*)kmalloc(len*sizeof(struct PathElem), GFP_KERNEL);
	for (j=0; j<len; ++j) {
		elems[len-j-1] = list->e;
		list = list->next;
	}
	path_buf->len = len;
	path_buf->path = elems;
	return 1;
}

int str_same(char *a, char *b) {
	int idx = 0;
	for (;;) {
		if (a[idx] == '\0' && b[idx] == '\0') {
			return 1;
		}
		if (a[idx] !=  b[idx]) {
			return 0;
		}
		++idx;
	}
}

int str_len(char* a) {
	int idx=0;
	while (a[idx++] != '\0') {}
	return idx;
}

struct JsonValue *get_value(struct JsonValue *json, struct PathElem *path, int len);

char* resolve_path_elem(struct JsonValue *json, struct PathElem path) {
	if (path.is_ref) {
		int i;
		if (path.path.len <= 0) return NULL;
		struct JsonValue* s = get_value(json, path.path.path, path.path.len);
		if (s == NULL || s->type != STRING) return NULL;
		return s->string.buf;
	}
	else {
		return path.name;
	}
}

struct JsonValue *get_value_sys_time(int len) {
	if (len != 0) return NULL;
	return NULL;
}

struct JsonValue *access(struct JsonValue *json, char* buf) {
	int i;
	for (i=0; i<json->pairs.len; ++i) {
		if (str_same(json->pairs.pairs[i].key, buf)) {
			return &json->pairs.pairs[i].value;
		}
	}
	return NULL;
}

struct JsonValue *get_out_value(struct JsonValue *json, struct PathElem *path, int len, int create_elems) {
	if (len <= 0) return json;
	char *head = resolve_path_elem(json, *path);
	int i;
	if (head == NULL) return NULL;
	if (json->type != OBJECT) return NULL;
	for (i=0; i<json->pairs.len; ++i) {
		if (str_same(json->pairs.pairs[i].key, head)) {
			return get_out_value(&json->pairs.pairs[i].value, path+1, len-1, create_elems);
		}
	}
	if (!create_elems) return NULL;
	if (json->pairs.len+1 > json->pairs.mem_len) {
		int i;
		struct Pair *pairs = (struct Pair*)kmalloc(sizeof(struct Pair)*json->pairs.mem_len*2+1, GFP_KERNEL);
		if (pairs == NULL) return NULL;
		for (i=0; i<json->pairs.len; ++i) {
			pairs[i] = json->pairs.pairs[i];
		}
		if(json->pairs.mem_len > 0)
			kfree(json->pairs.pairs);
		json->pairs.pairs = pairs;
		json->pairs.mem_len = json->pairs.mem_len*2+1;
	}
	struct JsonValue value;
	value.type = OBJECT;
	value.pairs.pairs = (struct Pair*)kmalloc(sizeof(struct Pair), GFP_KERNEL);
	value.pairs.len = 0;
	value.pairs.mem_len = 0;

	int head_len = str_len(head);
	char *key_buf = (char*)kmalloc(head_len, GFP_KERNEL);
	for (i = 0; i<head_len; ++i) {
		key_buf[i] = head[i];
	}
	json->pairs.pairs[json->pairs.len].key = key_buf;
	json->pairs.pairs[json->pairs.len].value = value;
	json->pairs.len++;
	return get_out_value(&json->pairs.pairs[json->pairs.len-1].value, path+1, len-1, create_elems);
}

struct JsonValue* get_value_sys(struct JsonValue *json, struct PathElem *path, int len) {
	if (len <= 0) return NULL;
	char *head = resolve_path_elem(json, *path);
	if (head == NULL) return NULL;
	if (str_same(head, "time"))
		return get_value_sys_time(len-1);
	return NULL;
}

struct JsonValue* get_value(struct JsonValue *json, struct PathElem *path, int len) {
	if (len <= 0) return NULL;
	char *head = resolve_path_elem(json, *path);
	if (head == NULL)
	if (head == NULL) return NULL;
	if (str_same(head, "out"))
		return get_out_value(json, path+1, len-1, 0);
	if (str_same(head, "sys"))
		return get_value_sys(json, path+1, len-1);
	return NULL;
}

void sweep(struct JsonValue *json) {
	int i;
	switch (json->type) {
		case INTEGER:
			return;
		case BOOLEAN:
			return;
		case STRING:
			return;
		case OBJECT:
			for (i=0; i<json->pairs.len; ++i) {
				kfree(json->pairs.pairs[i].key);
				sweep(&json->pairs.pairs[i].value);
			}
			return;
		case ARRAY:
			for (i=0; i<json->arrary.len; ++i) {
				sweep(json->arrary.arr + i);
			}
			return;
	}
}

struct JsonValue* eval(struct JsonValue* out, struct JsonValue *root) {
	if (root->type == OBJECT) {
		struct JsonValue *type = access(root, "type");
		if (type == NULL || type->type != STRING) return NULL;
		if (str_same(type->string.buf, "op")) {
			struct JsonValue *op = access(root, "op");
			if (op == NULL || op->type != STRING) return NULL;

			struct JsonValue *lhr = access(root, "lhr");
			if (lhr == NULL) return NULL;

			struct JsonValue *rhr = access(root, "rhr");
			if (rhr == NULL) return NULL;

			if (str_same(op->string.buf, "sub")) {
				struct JsonValue *lhr_evaluated = eval(out, lhr);
				struct JsonValue *rhr_evaluated = eval(out, rhr);
				if (lhr_evaluated == NULL ||
					lhr_evaluated->type != INTEGER ||
					rhr_evaluated == NULL ||
					rhr_evaluated->type != INTEGER)
					return NULL;
				struct JsonValue *json = (struct JsonValue*)kmalloc(sizeof(struct JsonValue), GFP_KERNEL);
				json->type = INTEGER;
				json->integer = lhr_evaluated->integer - rhr_evaluated->integer;
				return json;
			}
		}
		else if (str_same(type->string.buf, "ref")) {
			struct JsonValue *name = access(root, "name");
			if (name == NULL || name->type != STRING) return NULL;
			struct Path path;
			if (!parse_path(&path, name->string.buf, name->string.len))
				return NULL;
			return get_value(out, path.path, path.len);
		}
	}
	return root;
}

int exec(struct JsonValue* out, struct JsonValue *json) {
	struct JsonValue *type = access(json, "type");
	if (type == NULL || type->type != STRING) return 0;
	if (str_same(type->string.buf, "assign")) {
		struct JsonValue *target = access(json, "target");
		struct JsonValue *value = access(json, "value");
		if (value == NULL || target == NULL || target->type != STRING) return 0;
		struct Path path;
		if (!parse_path(&path, target->string.buf, target->string.len))
			return 0;
		if (path.len == 0 || path.path->is_ref || !str_same(path.path->name, "out")) NULL;
		struct JsonValue *target_ptr = get_out_value(out, path.path+1, path.len-1, 1);
		if (target_ptr == NULL) return 0;
		sweep(target_ptr);
		struct JsonValue *json = eval(out, value);
		if (json == NULL) return 0;
		*target_ptr = *json;
		sweep(json);
		return 1;
	}
	return 0;
}

MODULE_LICENSE("GPL");
