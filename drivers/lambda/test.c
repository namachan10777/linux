#include "json.h"
#include <stdio.h>

int main() {
	char buf[1024];
	ParseResult result;
	char src[] = "\"string \\\"hoge\\\"\"";
	result = parse(src);
	if (result.type != SUCCESS) return -1;
	printf("%d\n", result.value.type);
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);

	char src2[] = "-123";
	result = parse(src2);
	if (result.type != SUCCESS) return -1;
	printf("%d\n", result.value.type);
	printf("%d\n", result.value.integer);
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s\n", buf);
}
