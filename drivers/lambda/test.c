#include "json.h"
#include <stdio.h>

int main() {
	char buf[1024];
	ParseResult result;
	char src[] = "\"string \\\"hoge\\\"\"";
	result = parse(src);
	if (result.type != SUCCESS) return -1;
	if (result.used != 16) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src2[] = "-123";
	result = parse(src2);
	if (result.type != SUCCESS) return -1;
	printf("%d\n", result.used);
	if (result.used != 4) return -1;
	stringify(buf, sizeof(buf), result.value);
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src3[] = "true";
	result = parse(src3);
	if (result.type != SUCCESS) return -1;
	if (result.used != 4) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src4[] = "false";
	result = parse(src4);
	if (result.type != SUCCESS) return -1;
	if (result.used != 5) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src5[] = "[]";
	result = parse(src5);
	if (result.type != SUCCESS) return -1;
	if (result.used != 2) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src6[] = "[1,2,3]";
	result = parse(src6);
	if (result.type != SUCCESS) return -1;
	if (result.used != 7) {
		return -1;
	}
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);
}
