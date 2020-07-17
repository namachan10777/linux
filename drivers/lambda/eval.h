#ifndef __EVAL_H__
#define __EVAL_H__
#include "json.h"

#define STRING(name, inner) \
	struct String name; \
	char name ## _buf[] = inner; \
	name.buf = name ## _buf; \
	name.len = sizeof(inner) - 1;

int streq(struct String s1, struct String s2) {
	if (s1.len != s2.len) return 0;
	for (int i=0; i<s1.len; ++i) {
		if (s1.buf[i] != s2.buf[i]) return 0;
	}
	return 1;
}

int get_value(struct JsonValue *buf, struct JsonValue json, struct String key) {
	if (json.type != OBJECT)
		return 0;
	int i;
	for (i = 0; i<json.pairs.len; ++i) {
		if (streq(json.pairs.pairs[i].key, key)) {
			*buf = json.pairs.pairs[i].value;
			return 1;
		}
	}
	return 0;
}


int eval(struct JsonValue *out, struct JsonValue json) {
	if (json.type == OBJECT) {
		STRING(str_type, "type");
		struct JsonValue buf;
		if (!get_value(&buf, json, str_type) || buf.type != STRING)
			return 0;
		STRING(str_op, "op");
		STRING(str_int, "int");
		if (streq(buf.string, str_op)) {
			STRING(str_plus, "+");
			STRING(str_lhr, "lhr");
			STRING(str_rhr, "rhr");
			if (!get_value(&buf, json, str_op) || buf.type != STRING)
				return 0;
			if (streq(buf.string, str_plus)) {
				struct JsonValue lhr, rhr;
				if (!get_value(&rhr, json, str_rhr) || !get_value(&lhr, json, str_lhr))
					return 0;
				struct JsonValue lhr_evaluated, rhr_evaluated;
				if (!eval(&lhr_evaluated, lhr) || !eval(&rhr_evaluated, rhr))
					return 0;
				if (lhr_evaluated.type != INTEGER || rhr_evaluated.type != INTEGER)
					return 0;
				out->type = INTEGER;
				out->integer = lhr_evaluated.integer + rhr_evaluated.integer;
				return 1;
			}
		}
	}
	else if (json.type == INTEGER) {
		*out = json;
		return 1;
	}
	return 0;
}

#endif
