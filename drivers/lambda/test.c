#include "json.h"
#include <stdio.h>

int main() {
	char buf[1024];
	ParseResult result;
	char src[] = "\"string \\\"hoge\\\"\"";
	result = parse(src);
	if (result.type != SUCCESS) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src2[] = "-123";
	result = parse(src2);
	if (result.type != SUCCESS) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src3[] = "true";
	result = parse(src3);
	if (result.type != SUCCESS) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src4[] = "false";
	result = parse(src4);
	if (result.type != SUCCESS) return -1;
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);
}
