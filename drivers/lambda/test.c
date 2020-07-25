#define USER_LAND
#include "json.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"

#define ASSERT(cond) if (!(cond)) { printf("assersion failure: %s\n", #cond);  return -1; }

int main() {
	char buf[1024];
	ParseResult result;


	char src1[] = " \"string \\\"hoge\\\"\"";
	result = parse(src1, sizeof(src1));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.used == sizeof(src1)-1);
	ASSERT(result.value.type == STRING);
	ASSERT(stringify(buf, sizeof(buf), result.value) == sizeof(src1)-1);
	ASSERT(strcmp(buf, "\"string \\\"hoge\\\"\"") == 0);

	char src2[] = "\"hog";
	result = parse(src2, sizeof(src2));
	ASSERT(result.type == ERROR);

	char src3[] = "\"hog\\";
	result = parse(src3, sizeof(src3));
	ASSERT(result.type == ERROR);

	char src4[] = " 123 ";
	result = parse(src4, sizeof(src4));
	ASSERT(result.value.type == INTEGER);
	ASSERT(result.type == SUCCESS);
	ASSERT(result.used == 4);
	ASSERT(result.value.integer == 123);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 4);
	ASSERT(strcmp(buf, "123") == 0);

	char src5[] = " -123 ";
	result = parse(src5, sizeof(src5));
	ASSERT(result.value.type == INTEGER);
	ASSERT(result.type == SUCCESS);
	ASSERT(result.used == 5);
	ASSERT(result.value.integer == -123)
	ASSERT(stringify(buf, sizeof(buf), result.value) == 5);
	ASSERT(strcmp(buf, "-123") == 0);


	char src6[] = " -";
	result = parse(src6, sizeof(src6));
	ASSERT(result.type == ERROR);

	char src7[] = "false";
	result = parse(src7, sizeof(src7));
	ASSERT(result.value.type == BOOLEAN);
	ASSERT(result.type == SUCCESS);
	ASSERT(result.used == 5);
	ASSERT(result.value.boolean == 0);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 6);
	ASSERT(strcmp(buf, "false") == 0);

	char src8[] = "true";
	result = parse(src8, sizeof(src8));
	ASSERT(result.value.type == BOOLEAN);
	ASSERT(result.type == SUCCESS);
	ASSERT(result.used == 4);
	ASSERT(result.value.boolean == 1);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 5);
	ASSERT(strcmp(buf, "true") == 0);

	char src9[] = "[]";
	result = parse(src9, sizeof(src9));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.value.type == ARRAY);
	ASSERT(result.used == 2);
	ASSERT(result.value.arrary.len == 0);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 3);
	ASSERT(strcmp(buf, "[]") == 0);

	char src10[] = " [\"hoge\" ] ";
	result = parse(src10, sizeof(src10));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.value.type == ARRAY);
	ASSERT(result.used == 10);
	ASSERT(result.value.arrary.len == 1);
	ASSERT(result.value.arrary.arr[0].type == STRING);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 9);
	ASSERT(strcmp(buf, "[\"hoge\"]") == 0);

	char src11[] = "[1, 2, 3 ]";
	result = parse(src11, sizeof(src11));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.value.type == ARRAY);
	ASSERT(result.used == 10);
	ASSERT(result.value.arrary.len == 3);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 8);
	ASSERT(strcmp(buf, "[1,2,3]") == 0);

	char src12[] = "{\"hoge\": 1}";
	result = parse(src12, sizeof(src12));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.value.type == OBJECT);
	ASSERT(result.used == 11);
	ASSERT(result.value.pairs.len == 1);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 11);
	ASSERT(strcmp(buf, "{\"hoge\":1}") == 0);

	char src13[] = "{\"hoge\": [1,2,3],\"foo\":{\"bar\": 1}}";
	result = parse(src13, sizeof(src13));
	ASSERT(result.type == SUCCESS);
	ASSERT(result.value.type == OBJECT);
	ASSERT(result.used == 34);
	ASSERT(result.value.pairs.len == 2);
	ASSERT(stringify(buf, sizeof(buf), result.value) == 33);
	ASSERT(strcmp(buf, "{\"hoge\":[1,2,3],\"foo\":{\"bar\":1}}") == 0);

	char src14[] = "{\"type\": \"op\", \"op\": \"sub\", \"lhr\": 1, \"rhr\": 2}";
	result = parse(src14, sizeof(src14));
	struct JsonValue *out = empty_object();
	struct JsonValue *evaluated = eval(out, &result.value);
	ASSERT(evaluated->type = INTEGER && evaluated->integer == -1);

	char src15[] = "{\"type\": \"assign\", \"target\": \"out.hoge\", \"value\":\"fuga\" }";
	result = parse(src15, sizeof(src15));
	ASSERT(exec(out, &result.value));
	stringify(buf, sizeof(buf), *out);
	printf("%s\n", buf);

	char src16[] = "{\"type\": \"assign\", \"target\": \"out[out.hoge]\", \"value\":\"1\" }";
	printf("-----------\n");
	result = parse(src16, sizeof(src16));
	ASSERT(exec(out, &result.value));
	stringify(buf, sizeof(buf), *out);
	printf("%s\n", buf);

	/*char src14[1024];
	int idx=0;
	FILE* fp = fopen("1", "r");
	while ((src14[idx++] = fgetc(fp)) != EOF) {}
	result = parse(src14, idx);
	stringify(buf, sizeof(buf), result.value);
	printf("%s\n", buf);*/
	return 0;
}
