#include "json.h"
#include <linux/module.h>

#define STRING(name, inner) struct String name = { inner, sizeof(inner)-1};

int streq(struct String s1, struct String s2) {
	int i;
	if (s1.len != s2.len) return 0;
	for (i=0; i<s1.len; ++i) {
		if (s1.buf[i] != s2.buf[i]) return 0;
	}
	return 1;
}

int get_value(struct JsonValue *buf, struct JsonValue json, struct String key) {
	int i;
	if (json.type != OBJECT)
		return 0;
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
		struct JsonValue buf;
		struct String str_type = { "type", sizeof("type")-1 };
		struct String str_op = { "op", sizeof("op")-1 };
		if (!get_value(&buf, json, str_type) || buf.type != STRING)
			return 0;
		if (streq(buf.string, str_op)) {
			struct String str_plus = { "+", sizeof("+")-1 };
			struct String str_lhr = { "lhr", sizeof("lhr")-1 };
			struct String str_rhr = { "rhr", sizeof("rhr")-1 };
			if (!get_value(&buf, json, str_op) || buf.type != STRING)
				return 0;
			if (streq(buf.string, str_plus)) {
				struct JsonValue lhr, rhr;
				struct JsonValue lhr_evaluated, rhr_evaluated;
				if (!get_value(&rhr, json, str_rhr) || !get_value(&lhr, json, str_lhr))
					return 0;
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


MODULE_LICENSE("GPL");
