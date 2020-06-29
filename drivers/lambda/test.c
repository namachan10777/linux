#include "json.h"
#include <stdio.h>

int main() {
	char src[] = "\"string \\\"hoge\\\"\"";
	ParseResult result = parse(src);
	if (result.type != SUCCESS) return -1;
	char buf[1024];
	printf("%d\n", result.value.type);
	stringify(buf, sizeof(buf), result.value);
	printf("stringified: %s", buf);
}
