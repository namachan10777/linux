typedef enum {
	STRING,
	INTEGER,
	BOOLEAN,
	OBJECT,
	ARRAY
} JSONType;

typedef struct JsonValue JSONValue;
typedef struct Pair PAIR;

struct String {
	char *buf;
	int len;
};

typedef struct {
	int len;
	struct JsonValue *arr;
} Array;

struct Pairs {
	int len;
	struct Pair *pairs;
};

struct JsonValue {
	JSONType type;
	union {
		struct String string;
		int integer;
		int boolean;
		struct Pairs pairs;
		Array arrary;
	};
};

struct Pair {
	struct String key;
	struct JsonValue value;
};

typedef enum {
	SUCCESS,
	ERROR
} ParseResultType;

typedef struct {
	ParseResultType type;
	int used;
	union {
		int pos;
		struct JsonValue value;
	};
} ParseResult;

ParseResult parse(const char *input);
void stringify(char *buf, int buf_size, JSONValue json);
