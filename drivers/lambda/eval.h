#ifndef __EVAL_H__
#define __EVAL_H__
#include "json.h"

struct JsonValue* eval(struct JsonValue *out, struct JsonValue *json);
int exec(struct JsonValue* out, struct JsonValue *json);
struct JsonValue* empty_object(void);
#endif
