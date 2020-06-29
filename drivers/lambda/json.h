typedef enum {
	STRING,
	INTEGER,
	OBJECT,
	ARRAY
} JSONType;

typedef struct JsonValue JSONValue;

typedef struct {
	char* key;
	JSONValue *value;
} Pair;

struct String {
	char *buf;
	int len;
};

struct JsonValue {
	JSONType type;
	union {
		struct String string;
		int integer;
		Pair *keyvalue;
		JSONValue *arrary;
	};
};

typedef enum {
	SUCCESS,
	ERROR
} ParseResultType;

typedef struct {
	ParseResultType type;
	union {
		int pos;
		struct JsonValue value;
	};
} ParseResult;

ParseResult parse(const char *input);
void stringify(char *buf, int buf_size, JSONValue json);
