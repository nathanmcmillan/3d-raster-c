/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

#define keyword_size(a, b)      \
    if (a != b)                 \
        return TOKEN_UNDEFINED; \
    else                        \
        return

#define new_undefined() ((Value){VALUE_UNDEFINED, {.i = 0}})
#define new_nil() ((Value){VALUE_NIL, {.i = 0}})
#define new_bool(v) ((Value){VALUE_BOOL, {.b = v}})
#define new_int(v) ((Value){VALUE_INTEGER, {.i = v}})
#define new_float(v) ((Value){VALUE_FLOAT, {.f = v}})
#define new_str(v) ((Value){VALUE_STRING, {.s = v}})
#define new_func(v) ((Value){VALUE_FUNC, {.func = v}})
#define new_native(v) ((Value){VALUE_FUNC_NATIVE, {.native = v}})

#define as_bool(v) ((v).as.b)
#define as_int(v) ((v).as.i)
#define as_float(v) ((v).as.f)
#define as_string(v) ((v).as.s)
#define as_func(v) ((v).as.func)
#define as_native(v) ((v).as.native)

#define is_undefined(v) ((v).is == VALUE_UNDEFINED)
#define is_nil(v) ((v).is == VALUE_NIL)
#define is_bool(v) ((v).is == VALUE_BOOL)
#define is_int(v) ((v).is == VALUE_INTEGER)
#define is_float(v) ((v).is == VALUE_FLOAT)
#define is_string(v) ((v).is == VALUE_STRING)
#define is_func(v) ((v).is == VALUE_FUNC)
#define is_native(v) ((v).is == VALUE_FUNC_NATIVE)

#define STR_NIL "Nil"
#define STR_TRUE "True"
#define STR_FALSE "False"

#define macro_arithmetic_op(_binary_)                           \
    Value b = machine_pop(this);                                \
    Value a = machine_pop(this);                                \
    if (is_int(a)) {                                            \
        if (is_int(b)) {                                        \
            a.as.i _binary_ b.as.i;                             \
            machine_push(this, a);                              \
        } else if (is_float(b)) {                               \
            b.as.f _binary_ a.as.i;                             \
            machine_push(this, a);                              \
        } else {                                                \
            machine_runtime_error(this, "Operand non-number."); \
            return;                                             \
        }                                                       \
    } else if (is_float(a)) {                                   \
        if (is_int(b)) {                                        \
            a.as.f _binary_ b.as.i;                             \
            machine_push(this, a);                              \
        } else if (is_float(b)) {                               \
            a.as.f _binary_ b.as.f;                             \
            machine_push(this, a);                              \
        } else {                                                \
            machine_runtime_error(this, "Operand non-number."); \
            return;                                             \
        }                                                       \
    } else {                                                    \
        machine_runtime_error(this, "Operand non-number.");     \
        return;                                                 \
    }

#define macro_compare_op(_compare_)                                                \
    Value b = machine_pop(this);                                                   \
    Value a = machine_pop(this);                                                   \
    if (is_int(a)) {                                                               \
        if (is_int(b)) {                                                           \
            machine_push(this, new_bool(as_int(a) _compare_ as_int(b)));           \
        } else if (is_float(b)) {                                                  \
            machine_push(this, new_bool((double)as_int(a) _compare_ as_float(b))); \
        } else {                                                                   \
            machine_runtime_error(this, "Comparing non-number.");                  \
            return;                                                                \
        }                                                                          \
    } else if (is_float(a)) {                                                      \
        if (is_int(b)) {                                                           \
            machine_push(this, new_bool(as_float(a) _compare_(double) as_int(b))); \
        } else if (is_float(b)) {                                                  \
            machine_push(this, new_bool(as_float(a) _compare_ as_float(b)));       \
        } else {                                                                   \
            machine_runtime_error(this, "Comparing non-number.");                  \
            return;                                                                \
        }                                                                          \
    } else {                                                                       \
        machine_runtime_error(this, "Comparing non-number.");                      \
        return;                                                                    \
    }

static const float LOAD_FACTOR = 0.80f;

static const unsigned int INITIAL_BINS = 1 << 3;

static const unsigned int MAXIMUM_BINS = 1 << 30;

enum ValueType {
    VALUE_BOOL,
    VALUE_FLOAT,
    VALUE_FUNC,
    VALUE_FUNC_NATIVE,
    VALUE_INTEGER,
    VALUE_LIST,
    VALUE_NIL,
    VALUE_STRING,
    VALUE_TABLE,
    VALUE_UNDEFINED,
};

enum TokenType {
    TOKEN_AND,
    TOKEN_ASSIGN,
    TOKEN_COMMA,
    TOKEN_CONST,
    TOKEN_BEGIN,
    TOKEN_DIVIDE,
    TOKEN_DOT,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_END,
    TOKEN_EOF,
    TOKEN_EQUAL,
    TOKEN_ERROR,
    TOKEN_FLOAT,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_IDENT,
    TOKEN_IF,
    TOKEN_USE,
    TOKEN_INTEGER,
    TOKEN_LEFT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_LINE,
    TOKEN_LEFT_PAREN,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SUBTRACT,
    TOKEN_MULTIPLY,
    TOKEN_NIL,
    TOKEN_NOT,
    TOKEN_NOT_EQUAL,
    TOKEN_OR,
    TOKEN_PASS,
    TOKEN_ADD,
    TOKEN_RIGHT_BRACE,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_RIGHT_PAREN,
    TOKEN_STRING,
    TOKEN_UNDEFINED,
    TOKEN_VALUE,
    TOKEN_LET,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_PRINT,
};

enum Precedence {
    PRECEDENCE_NONE,
    PRECEDENCE_ASSIGN,
    PRECEDENCE_OR,
    PRECEDENCE_AND,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_COMPARE,
    PRECEDENCE_TERM,
    PRECEDENCE_FACTOR,
    PRECEDENCE_UNARY,
    PRECEDENCE_CALL,
    PRECEDENCE_PRIMARY,
};

enum OpCode {
    OP_ADD,
    OP_DIVIDE,
    OP_CONSTANT,
    OP_CONSTANT_TWO,
    OP_MULTIPLY,
    OP_NEGATE,
    OP_SUBTRACT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_RETURN,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
};

typedef struct Value Value;
typedef struct Token Token;
typedef struct Local Local;
typedef struct Rule Rule;
typedef struct Script Script;
typedef struct ValueMapItem ValueMapItem;
typedef struct ValueMap ValueMap;
typedef struct ValuePool ValuePool;
typedef struct ByteCode ByteCode;
typedef struct Function Function;
typedef struct NativeFunction NativeFunction;
typedef struct Frame Frame;
typedef struct Machine Machine;
typedef struct Scope Scope;
typedef struct Compiler Compiler;

static void compile_with_precedence(Compiler *this, enum Precedence precedence);
static void compile_call(Compiler *this, bool assign);
static void compile_group(Compiler *this, bool assign);
static void compile_true(Compiler *this, bool assign);
static void compile_false(Compiler *this, bool assign);
static void compile_nil(Compiler *this, bool assign);
static void compile_number(Compiler *this, bool assign);
static void compile_string(Compiler *this, bool assign);
static void compile_variable(Compiler *this, bool assign);
static void compile_unary(Compiler *this, bool assign);
static void compile_binary(Compiler *this, bool assign);
static void compile_and(Compiler *this, bool assign);
static void compile_or(Compiler *this, bool assign);
static void declaration(Compiler *this);
static void statement(Compiler *this);
static void print_statement(Compiler *this);
static void expression_statement(Compiler *this);
static void expression(Compiler *this);

struct Value {
    enum ValueType is;
    union {
        bool b;
        i64 i;
        double f;
        String *s;
        Function *func;
        NativeFunction *native;
    } as;
};

struct Token {
    enum TokenType type;
    int row;
    int column;
    usize start;
    int length;
};

struct Local {
    Token name;
    int depth;
    bool constant;
};

struct Rule {
    void (*prefix)(Compiler *, bool);
    void (*infix)(Compiler *, bool);
    enum Precedence precedence;
};

struct Script {
    const char *name;
    Value **variables;
    usize variable_count;
};

struct ValueMapItem {
    usize hash;
    String *key;
    Value value;
    ValueMapItem *next;
};

struct ValueMap {
    unsigned int size;
    unsigned int bins;
    ValueMapItem **items;
};

struct ValuePool {
    int count;
    int capacity;
    Value *values;
};

struct ByteCode {
    int count;
    int capacity;
    u8 *instructions;
    int *rows;
    ValuePool constants;
};

struct Function {
    String *name;
    int arity;
    ByteCode code;
};

typedef Value (*NativeCall)(int count, Value *arguments);

struct NativeFunction {
    String *name;
    NativeCall native;
};

struct Frame {
    Function *func;
    usize ip;
    usize stack_top;
};

struct Machine {
    Value stack[HYMN_STACK_MAX];
    usize stack_top;
    Frame frames[HYMN_FRAMES_MAX];
    int frame_count;
    ValueMap strings;
    ValueMap globals;
    String *error;
};

struct Scope {
    struct Scope *enclosing;
    Function *func;
    enum FunctionType type;
    Local locals[UINT8_COUNT];
    int local_count;
    int depth;
};

struct Compiler {
    usize pos;
    int row;
    int column;
    char *source;
    usize size;
    Token alpha;
    Token beta;
    Token gamma;
    Machine *machine;
    Scope *scope;
    bool panic;
    String *error;
};

Rule rules[] = {
    [TOKEN_ADD] = {NULL, compile_binary, PRECEDENCE_TERM},
    [TOKEN_AND] = {NULL, compile_and, PRECEDENCE_AND},
    [TOKEN_ASSIGN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_BEGIN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CONST] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_DIVIDE] = {NULL, compile_binary, PRECEDENCE_FACTOR},
    [TOKEN_DOT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELIF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_END] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EOF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY},
    [TOKEN_ERROR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FALSE] = {compile_false, NULL, PRECEDENCE_NONE},
    [TOKEN_FLOAT] = {compile_number, NULL, PRECEDENCE_NONE},
    [TOKEN_FOR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FUNCTION] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_GREATER] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_GREATER_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_IDENT] = {compile_variable, NULL, PRECEDENCE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_INTEGER] = {compile_number, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_BRACKET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LINE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_PAREN] = {compile_group, compile_call, PRECEDENCE_CALL},
    [TOKEN_LESS] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_LESS_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_MULTIPLY] = {NULL, compile_binary, PRECEDENCE_FACTOR},
    [TOKEN_NIL] = {compile_nil, NULL, PRECEDENCE_NONE},
    [TOKEN_NOT] = {compile_unary, NULL, PRECEDENCE_NONE},
    [TOKEN_NOT_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY},
    [TOKEN_OR] = {NULL, compile_or, PRECEDENCE_OR},
    [TOKEN_PASS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_BRACKET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_STRING] = {compile_string, NULL, PRECEDENCE_NONE},
    [TOKEN_SUBTRACT] = {compile_unary, compile_binary, PRECEDENCE_TERM},
    [TOKEN_TRUE] = {compile_true, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PRECEDENCE_NONE},
};

static usize string_hashcode(String *key) {
    usize length = string_len(key);
    usize hash = 0;
    for (usize i = 0; i < length; i++) {
        hash = 31 * hash + (usize)key[i];
    }
    return hash;
}

static void map_init(ValueMap *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(ValueMapItem *));
}

static unsigned int map_get_bin(ValueMap *this, usize hash) {
    return (this->bins - 1) & hash;
}

static usize map_hash_mix(usize hash) {
    return hash ^ (hash >> 16);
}

static void map_resize(ValueMap *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    ValueMapItem **old_items = this->items;
    ValueMapItem **items = safe_calloc(bins, sizeof(ValueMapItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        ValueMapItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->hash] = item;
        } else {
            ValueMapItem *low_head = NULL;
            ValueMapItem *low_tail = NULL;
            ValueMapItem *high_head = NULL;
            ValueMapItem *high_tail = NULL;
            do {
                if ((old_bins & item->hash) == 0) {
                    if (low_tail == NULL) {
                        low_head = item;
                    } else {
                        low_tail->next = item;
                    }
                    low_tail = item;
                } else {
                    if (high_tail == NULL) {
                        high_head = item;
                    } else {
                        high_tail->next = item;
                    }
                    high_tail = item;
                }
                item = item->next;
            } while (item != NULL);

            if (low_tail != NULL) {
                low_tail->next = NULL;
                items[i] = low_head;
            }

            if (high_tail != NULL) {
                high_tail->next = NULL;
                items[i + old_bins] = high_head;
            }
        }
    }

    free(old_items);

    this->bins = bins;
    this->items = items;
}

static void map_put(ValueMap *this, String *key, Value value) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    ValueMapItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            item->value = value;
            return;
        }
        previous = item;
        item = item->next;
    }
    item = safe_malloc(sizeof(ValueMapItem));
    item->hash = hash;
    item->key = key;
    item->value = value;
    item->next = NULL;
    if (previous == NULL) {
        this->items[bin] = item;
    } else {
        previous->next = item;
    }
    this->size++;
    if (this->size >= this->bins * LOAD_FACTOR) {
        map_resize(this);
    }
}

static Value map_get(ValueMap *this, String *key) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            return item->value;
        }
        item = item->next;
    }
    return new_undefined();
}

static Value map_remove(ValueMap *this, String *key) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    ValueMapItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            this->size -= 1;
            return item->value;
        }
        previous = item;
        item = item->next;
    }
    return new_undefined();
}

static void map_clear(ValueMap *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        ValueMapItem *item = this->items[i];
        while (item != NULL) {
            ValueMapItem *next = item->next;
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static bool map_is_empty(ValueMap *this) {
    return this->size == 0;
}

static bool map_not_empty(ValueMap *this) {
    return this->size != 0;
}

static unsigned int map_size(ValueMap *this) {
    return this->size;
}

static void map_delete(ValueMap *this) {
    map_clear(this);
    free(this->items);
}

static const char *value_name(enum ValueType value) {
    switch (value) {
    case VALUE_UNDEFINED: return "UNDEFINED";
    case VALUE_NIL: return "NIL";
    case VALUE_BOOL: return "BOOLEAN";
    case VALUE_INTEGER: return "INTEGER";
    case VALUE_FLOAT: return "FLOAT";
    case VALUE_STRING: return "STRING";
    case VALUE_FUNC: return "FUNCTION";
    case VALUE_FUNC_NATIVE: return "NATIVE_FUNCTION";
    case VALUE_LIST: return "LIST";
    case VALUE_TABLE: return "TABLE";
    default: return "VALUE ?";
    }
}

static const char *token_name(enum TokenType type) {
    switch (type) {
    case TOKEN_USE: return "USE";
    case TOKEN_FOR: return "FOR";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_AND: return "AND";
    case TOKEN_OR: return "OR";
    case TOKEN_FLOAT: return "FLOAT";
    case TOKEN_INTEGER: return "INTEGER";
    case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
    case TOKEN_ADD: return "ADD";
    case TOKEN_SUBTRACT: return "SUBTRACT";
    case TOKEN_MULTIPLY: return "MULTIPLY";
    case TOKEN_DIVIDE: return "DIVIDE";
    case TOKEN_IDENT: return "IDENT";
    case TOKEN_FUNCTION: return "FUNCTION";
    case TOKEN_EOF: return "EOF";
    case TOKEN_IF: return "IF";
    case TOKEN_ELSE: return "ELSE";
    case TOKEN_ELIF: return "ELIF";
    case TOKEN_BEGIN: return "BEGIN";
    case TOKEN_END: return "END";
    case TOKEN_TRUE: return "TRUE";
    case TOKEN_FALSE: return "FALSE";
    case TOKEN_NIL: return "NIL";
    case TOKEN_NOT: return "NOT";
    case TOKEN_EQUAL: return "EQUAL";
    case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
    case TOKEN_LESS: return "LESS";
    case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
    case TOKEN_GREATER: return "GREATER";
    case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
    case TOKEN_STRING: return "STRING";
    case TOKEN_PRINT: return "PRINT";
    case TOKEN_ASSIGN: return "ASSIGN";
    case TOKEN_LET: return "LET";
    case TOKEN_CONST: return "CONST";
    case TOKEN_COLON: return "COLON";
    case TOKEN_SEMICOLON: return "SEMICOLON";
    case TOKEN_RETURN: return "RETURN";
    default: return "?";
    }
}

static void debug_value(Value value) {
    printf("%s: ", value_name(value.is));
    switch (value.is) {
    case VALUE_UNDEFINED: printf("UNDEFINED"); break;
    case VALUE_NIL: printf("NIL"); break;
    case VALUE_BOOL: printf("%s", as_bool(value) ? "TRUE" : "FALSE"); break;
    case VALUE_INTEGER: printf("%" PRId64, as_int(value)); break;
    case VALUE_FLOAT: printf("%g", as_float(value)); break;
    case VALUE_STRING: printf("\"%s\"", as_string(value)); break;
    case VALUE_FUNC: printf("<%s>", as_func(value)->name); break;
    case VALUE_FUNC_NATIVE: printf("<%s>", as_native(value)->name); break;
    default: printf("?");
    }
}

static void compiler_delete(Compiler *this) {
    string_delete(this->error);
}

static inline ByteCode *current(Compiler *this) {
    return &this->scope->func->code;
}

static void compile_error(Compiler *this, Token *token, const char *format, ...) {
    if (this->panic) {
        return;
    }
    this->panic = true;

    if (this->error == NULL) {
        this->error = new_string("");
    }

    this->error = string_append_format(this->error, "[Line %d] Error", token->row);
    if (token->type == TOKEN_EOF) {
        this->error = string_append(this->error, ": ");
    } else {
        this->error = string_append_format(this->error, " at '%.*s': ", token->length, &this->source[token->start]);
    }

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    this->error = string_append(this->error, chars);
    free(chars);

    this->error = string_append_char(this->error, '\n');
}

static char next_char(Compiler *this) {
    usize pos = this->pos;
    if (pos == this->size) {
        return '\0';
    }
    char c = this->source[pos];
    this->pos = pos + 1;
    if (c == '\n') {
        this->row++;
        this->column = 0;
    } else {
        this->column++;
    }
    return c;
}

static char peek_char(Compiler *this) {
    if (this->pos == this->size) {
        return '\0';
    }
    return this->source[this->pos];
}

static void token(Compiler *this, enum TokenType type) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s\n", token_name(type));
#endif
    Token *gamma = &this->gamma;
    gamma->type = type;
    gamma->row = this->row;
    gamma->column = this->column;
}

static void value_token(Compiler *this, enum TokenType type, usize start, usize end) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s: %.*s\n", token_name(type), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->type = type;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static void string_token(Compiler *this, usize start, usize end) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s, \"%.*s\"\n", token_name(TOKEN_STRING), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->type = TOKEN_STRING;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static enum TokenType ident_trie(char *ident, int offset, const char *rest, enum TokenType type) {
    int i = 0;
    do {
        if (ident[offset + i] != rest[i]) {
            return TOKEN_UNDEFINED;
        }
        i++;
    } while (rest[i] != '\0');
    return type;
}

static enum TokenType ident_keyword(char *ident, usize size) {
    switch (ident[0]) {
    case 'o': keyword_size(size, 2) ident_trie(ident, 1, "r", TOKEN_OR);
    case 'i': keyword_size(size, 2) ident_trie(ident, 1, "f", TOKEN_IF);
    case 'u': keyword_size(size, 3) ident_trie(ident, 1, "se", TOKEN_USE);
    case 'a': keyword_size(size, 3) ident_trie(ident, 1, "nd", TOKEN_AND);
    case 'n': keyword_size(size, 3) ident_trie(ident, 1, "il", TOKEN_NIL);
    case 'l': keyword_size(size, 3) ident_trie(ident, 1, "et", TOKEN_LET);
    case 't': keyword_size(size, 4) ident_trie(ident, 1, "rue", TOKEN_TRUE);
    case 'c': keyword_size(size, 5) ident_trie(ident, 1, "onst", TOKEN_CONST);
    case 'w': keyword_size(size, 5) ident_trie(ident, 1, "hile", TOKEN_WHILE);
    case 'p': keyword_size(size, 5) ident_trie(ident, 1, "rint", TOKEN_PRINT);
    case 'b': keyword_size(size, 5) ident_trie(ident, 1, "egin", TOKEN_BEGIN);
    case 'r': keyword_size(size, 6) ident_trie(ident, 1, "eturn", TOKEN_RETURN);
    case 'e':
        if (size == 3) {
            return ident_trie(ident, 1, "nd", TOKEN_END);
        } else if (size == 4 && ident[1] == 'l') {
            switch (ident[2]) {
            case 's': return ident_trie(ident, 3, "e", TOKEN_ELSE);
            case 'i': return ident_trie(ident, 3, "f", TOKEN_ELIF);
            }
        }
        break;
    case 'f':
        if (size == 3) {
            return ident_trie(ident, 1, "or", TOKEN_FOR);
        } else if (size == 5) {
            return ident_trie(ident, 1, "alse", TOKEN_FALSE);
        } else if (size == 8) {
            return ident_trie(ident, 1, "unction", TOKEN_FUNCTION);
        }
        break;
    }
    return TOKEN_UNDEFINED;
}

static void push_ident_token(Compiler *this, usize start, usize end) {
    char *ident = &this->source[start];
    usize size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        token(this, keyword);
        return;
    }
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s, %.*s\n", token_name(TOKEN_IDENT), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = TOKEN_IDENT;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static bool is_digit(char c) {
    return '0' <= c and c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or c == '_';
}

static void advance(Compiler *this) {
    this->alpha = this->beta;
    this->beta = this->gamma;
    if (this->beta.type == TOKEN_EOF) {
        return;
    }
    while (true) {
        char c = next_char(this);
        switch (c) {
        case '#':
            c = peek_char(this);
            while (c != '\n' and c != '\0') {
                next_char(this);
                c = peek_char(this);
            }
            continue;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            c = peek_char(this);
            while (c != '\0' and (c == ' ' or c == '\t' or c == '\r' or c == '\n')) {
                next_char(this);
                c = peek_char(this);
            }
            continue;
        case '!':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_NOT_EQUAL);
            } else {
                token(this, TOKEN_NOT);
            }
            return;
        case '=':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_EQUAL);
            } else {
                token(this, TOKEN_ASSIGN);
            }
            return;
        case '>':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_GREATER_EQUAL);
            } else {
                token(this, TOKEN_GREATER);
            }
            return;
        case '<':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_LESS_EQUAL);
            } else {
                token(this, TOKEN_LESS);
            }
            return;
        case '+': token(this, TOKEN_ADD); return;
        case '-': token(this, TOKEN_SUBTRACT); return;
        case '*': token(this, TOKEN_MULTIPLY); return;
        case '/': token(this, TOKEN_DIVIDE); return;
        case ',': token(this, TOKEN_COMMA); return;
        case '.': token(this, TOKEN_DOT); return;
        case '(': token(this, TOKEN_LEFT_PAREN); return;
        case ')': token(this, TOKEN_RIGHT_PAREN); return;
        case '[': token(this, TOKEN_LEFT_BRACKET); return;
        case ']': token(this, TOKEN_RIGHT_BRACKET); return;
        case '{': token(this, TOKEN_LEFT_BRACE); return;
        case '}': token(this, TOKEN_RIGHT_BRACE); return;
        case ':': token(this, TOKEN_COLON); return;
        case ';': token(this, TOKEN_SEMICOLON); return;
        case '\0': token(this, TOKEN_EOF); return;
        case '"': {
            usize start = this->pos;
            while (true) {
                c = next_char(this);
                if (c == '"' or c == '\0') {
                    break;
                }
            }
            usize end = this->pos - 1;
            string_token(this, start, end);
            return;
        }
        default: {
            if (is_digit(c)) {
                usize start = this->pos - 1;
                while (is_digit(peek_char(this))) {
                    next_char(this);
                }
                bool discrete = true;
                if (peek_char(this) == '.') {
                    discrete = false;
                    next_char(this);
                    while (is_digit(peek_char(this))) {
                        next_char(this);
                    }
                }
                usize end = this->pos;
                if (discrete) {
                    value_token(this, TOKEN_INTEGER, start, end);
                } else {
                    value_token(this, TOKEN_FLOAT, start, end);
                }
                return;
            } else if (is_ident(c)) {
                usize start = this->pos - 1;
                while (is_ident(peek_char(this))) {
                    next_char(this);
                }
                usize end = this->pos;
                push_ident_token(this, start, end);
                return;
            } else {
                compile_error(this, &this->beta, "Unknown character '%c'", c);
            }
        }
        }
    }
}

static void value_pool_init(ValuePool *this) {
    this->count = 0;
    this->capacity = 8;
    this->values = safe_malloc(8 * sizeof(Value));
}

static void value_pool_delete(ValuePool *this) {
    free(this->values);
}

static void value_pool_add(ValuePool *this, Value value) {
    if (this->count + 1 > this->capacity) {
        this->capacity *= 2;
        this->values = safe_realloc(this->values, this->capacity * sizeof(Value));
    }
    this->values[this->count] = value;
    this->count++;
}

static void byte_code_init(ByteCode *this) {
    this->count = 0;
    this->capacity = 8;
    this->instructions = safe_malloc(8 * sizeof(u8));
    this->rows = safe_malloc(8 * sizeof(int));
    value_pool_init(&this->constants);
}

static Function *new_function() {
    Function *func = safe_malloc(sizeof(Function));
    func->arity = 0;
    func->name = NULL;
    byte_code_init(&func->code);
    return func;
}

static NativeFunction *new_native_function(String *name, NativeCall native) {
    NativeFunction *func = safe_malloc(sizeof(NativeFunction));
    func->name = name;
    func->native = native;
    return func;
}

static void compiler_scope_init(Compiler *this, Scope *scope, enum FunctionType type) {
    scope->enclosing = this->scope;
    this->scope = scope;

    scope->local_count = 0;
    scope->depth = 0;
    scope->func = new_function();
    scope->type = type;

    if (type != TYPE_SCRIPT) {
        scope->func->name = new_string_from_substring(this->source, this->alpha.start, this->alpha.start + this->alpha.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static inline Compiler new_compiler(char *source, Machine *machine, Scope *scope) {
    Compiler this = {0};
    this.row = 1;
    this.column = 1;
    this.source = source;
    this.size = strlen(source);
    this.alpha.type = TOKEN_UNDEFINED;
    this.beta.type = TOKEN_UNDEFINED;
    this.gamma.type = TOKEN_UNDEFINED;
    this.machine = machine;
    compiler_scope_init(&this, scope, TYPE_SCRIPT);
    return this;
}

static void byte_code_delete(ByteCode *this) {
    free(this->instructions);
    free(this->rows);
    value_pool_delete(&this->constants);
}

static int byte_code_add_constant(ByteCode *this, Value value) {
    value_pool_add(&this->constants, value);
    return this->constants.count - 1;
}

static void write_op(ByteCode *this, u8 b, int row) {
    int count = this->count;
    if (count + 1 > this->capacity) {
        this->capacity *= 2;
        this->instructions = safe_realloc(this->instructions, this->capacity * sizeof(u8));
        this->rows = safe_realloc(this->rows, this->capacity * sizeof(int));
    }
    this->instructions[count] = b;
    this->rows[count] = row;
    this->count = count + 1;
}

static void write_two_op(ByteCode *this, u8 b, u8 n, int row) {
    int count = this->count;
    while (count + 2 > this->capacity) {
        this->capacity *= 2;
        this->instructions = safe_realloc(this->instructions, this->capacity * sizeof(u8));
        this->rows = safe_realloc(this->rows, this->capacity * sizeof(int));
    }
    this->instructions[count] = b;
    this->instructions[count + 1] = n;
    this->rows[count] = row;
    this->rows[count + 1] = row;
    this->count = count + 2;
}

static void write_constant(ByteCode *this, Value value, int row) {
    u8 constant = (u8)byte_code_add_constant(this, value);
    write_two_op(this, OP_CONSTANT, constant, row);
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static String *intern_string(ValueMap *this, String *value) {
    Value exists = map_get(this, value);
    if (is_undefined(exists)) {
        map_put(this, value, new_str(value));
        return NULL;
    } else {
        return as_string(exists);
    }
}

static Value machine_intern_string(Machine *this, String *value) {
    String *intern = intern_string(&this->strings, value);
    if (intern != NULL) {
        string_delete(value);
        return new_str(intern);
    }
    return new_str(value);
}

static bool check(Compiler *this, enum TokenType type) {
    return this->beta.type == type;
}

static bool match(Compiler *this, enum TokenType type) {
    if (!check(this, type)) {
        return false;
    }
    advance(this);
    return true;
}

static inline void emit(Compiler *this, u8 b) {
    write_op(current(this), b, this->alpha.row);
}

static inline void emit_two(Compiler *this, u8 b, u8 n) {
    write_two_op(current(this), b, n, this->alpha.row);
}

static void compile_with_precedence(Compiler *this, enum Precedence precedence) {
    advance(this);
    Rule *rule = token_rule(this->alpha.type);
    void (*prefix)(Compiler *, bool) = rule->prefix;
    if (prefix == NULL) {
        compile_error(this, &this->alpha, "Expected expression.");
        return;
    }
    bool assign = precedence <= PRECEDENCE_ASSIGN;
    prefix(this, assign);
    while (precedence <= token_rule(this->beta.type)->precedence) {
        advance(this);
        void (*infix)(Compiler *, bool) = token_rule(this->alpha.type)->infix;
        if (infix == NULL) {
            compile_error(this, &this->alpha, "No infix rule found.");
            return;
        }
        infix(this, assign);
    }
    if (assign && match(this, TOKEN_ASSIGN)) {
        compile_error(this, &this->beta, "Invalid assignment target.");
    }
}

static void consume(Compiler *this, enum TokenType type, const char *error) {
    if (this->beta.type == type) {
        advance(this);
        return;
    }
    compile_error(this, &this->beta, error);
}

static u8 arguments(Compiler *this) {
    u8 count = 0;
    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            expression(this);
            if (count == 255) {
                compile_error(this, &this->alpha, "Can't have more than 255 function arguments.");
                break;
            }
            count++;
        } while (match(this, TOKEN_COMMA));
    }
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments.");
    return count;
}

static void compile_call(Compiler *this, bool assign) {
    (void)assign;
    u8 args = arguments(this);
    emit_two(this, OP_CALL, args);
}

static void compile_group(Compiler *this, bool assign) {
    (void)assign;
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected right parenthesis.");
}

static void compile_true(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_TRUE);
}

static void compile_false(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_FALSE);
}

static void compile_nil(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_NIL);
}

static void compile_number(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->alpha;
    switch (alpha->type) {
    case TOKEN_INTEGER: {
        i64 number = (i64)strtoll(&this->source[alpha->start], NULL, 10);
        write_constant(current(this), new_int(number), alpha->row);
        break;
    }
    case TOKEN_FLOAT: {
        double number = strtod(&this->source[alpha->start], NULL);
        write_constant(current(this), new_float(number), alpha->row);
        break;
    }
    default:
        compile_error(this, alpha, "Expected a number");
    }
}

static void compile_string(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->alpha;
    String *s = new_string_from_substring(this->source, alpha->start, alpha->start + alpha->length);
    write_constant(current(this), machine_intern_string(this->machine, s), alpha->row);
}

static void function_delete(Function *this) {
    byte_code_delete(&this->code);
    free(this);
}

static void native_function_delete(NativeFunction *this) {
    free(this);
}

static void panic_halt(Compiler *this) {
    this->panic = false;
    while (true) {
        switch (this->beta.type) {
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
        case TOKEN_BEGIN:
        case TOKEN_EOF:
            return;
        }
        advance(this);
    }
}

static void begin_scope(Compiler *this) {
    this->scope->depth++;
}

static void end_scope(Compiler *this) {
    Scope *scope = this->scope;
    scope->depth--;
    while (scope->local_count > 0 && scope->locals[scope->local_count - 1].depth > scope->depth) {
        emit(this, OP_POP);
        scope->local_count--;
    }
}

static u8 ident_constant(Compiler *this, Token *token) {
    String *s = new_string_from_substring(this->source, token->start, token->start + token->length);
    return (u8)byte_code_add_constant(current(this), machine_intern_string(this->machine, s));
}

static void push_local(Compiler *this, Token name, bool constant) {
    Scope *scope = this->scope;
    if (scope->local_count == UINT8_COUNT) {
        compile_error(this, &name, "Too many local variables in scope.");
        return;
    }
    Local *local = &scope->locals[scope->local_count++];
    local->name = name;
    local->constant = constant;
    local->depth = -1;
}

static bool ident_match(Compiler *this, Token *a, Token *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(&this->source[a->start], &this->source[b->start], a->length) == 0;
}

static u8 variable(Compiler *this, bool constant, const char *error) {
    consume(this, TOKEN_IDENT, error);

    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return ident_constant(this, &this->alpha);
    }
    Token *name = &this->alpha;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (local->depth != -1 && local->depth < scope->depth) {
            break;
        } else if (ident_match(this, name, &local->name)) {
            compile_error(this, name, "Variable already exists in this scope.");
        }
    }
    push_local(this, *name, constant);
    return 0;
}

static void local_initialize(Compiler *this) {
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return;
    }
    scope->locals[scope->local_count - 1].depth = scope->depth;
}

static void define_variable(Compiler *this, u8 global) {
    if (this->scope->depth > 0) {
        local_initialize(this);
        return;
    }
    emit_two(this, OP_DEFINE_GLOBAL, global);
}

static void declare_new_variable(Compiler *this, bool constant) {
    u8 global = variable(this, constant, "Expected variable name.");
    consume(this, TOKEN_ASSIGN, "Expected '=' after variable");
    expression(this);
    define_variable(this, global);
}

static int resolve_local(Compiler *this, Token *name, bool *constant) {
    Scope *scope = this->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(this, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(this, name, "Can't reference local variable before initializing.");
            }
            *constant = local->constant;
            return i;
        }
    }
    return -1;
}

static void named_variable(Compiler *this, Token token, bool assign) {
    u8 get;
    u8 set;
    bool constant = false;
    int var = resolve_local(this, &token, &constant);
    if (var != -1) {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    } else {
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
        var = ident_constant(this, &token);
        // todo: const for globals
        // globals are evaluated at runtime
        // during compile they're literally just the string reference
        // this will need to change in order to store that it's const or not
    }
    if (assign && match(this, TOKEN_ASSIGN)) {
        if (constant) {
            compile_error(this, &token, "Constant variable can't be modified.");
        }
        expression(this);
        emit(this, set);
    } else {
        emit(this, get);
    }
    emit(this, (u8)var);
}

static void compile_variable(Compiler *this, bool assign) {
    named_variable(this, this->alpha, assign);
}

static void compile_unary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->alpha.type;
    compile_with_precedence(this, PRECEDENCE_UNARY);
    switch (type) {
    case TOKEN_NOT: emit(this, OP_NOT); break;
    case TOKEN_SUBTRACT: emit(this, OP_NEGATE); break;
    }
}

static void compile_binary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->alpha.type;
    Rule *rule = token_rule(type);
    compile_with_precedence(this, (enum Precedence)(rule->precedence + 1));
    switch (type) {
    case TOKEN_ADD: emit(this, OP_ADD); break;
    case TOKEN_SUBTRACT: emit(this, OP_SUBTRACT); break;
    case TOKEN_MULTIPLY: emit(this, OP_MULTIPLY); break;
    case TOKEN_DIVIDE: emit(this, OP_DIVIDE); break;
    case TOKEN_EQUAL: emit(this, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: emit(this, OP_NOT_EQUAL); break;
    case TOKEN_LESS: emit(this, OP_LESS); break;
    case TOKEN_LESS_EQUAL: emit(this, OP_LESS_EQUAL); break;
    case TOKEN_GREATER: emit(this, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emit(this, OP_GREATER_EQUAL); break;
    }
}

static int jump_instruction(Compiler *this, u8 instruction) {
    emit(this, instruction);
    emit_two(this, UINT8_MAX, UINT8_MAX);
    return current(this)->count - 2;
}

static void patch_jump(Compiler *this, int jump) {
    ByteCode *code = current(this);
    int offset = code->count - jump - 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->alpha, "Jump offset too large.");
        return;
    }
    code->instructions[jump] = (offset >> 8) & UINT8_MAX;
    code->instructions[jump + 1] = offset & UINT8_MAX;
}

static void compile_and(Compiler *this, bool assign) {
    (void)assign;
    int jump = jump_instruction(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);
    compile_with_precedence(this, PRECEDENCE_AND);
    patch_jump(this, jump);
}

static void compile_or(Compiler *this, bool assign) {
    (void)assign;
    int jump_else = jump_instruction(this, OP_JUMP_IF_FALSE);
    int jump = jump_instruction(this, OP_JUMP);
    patch_jump(this, jump_else);
    emit(this, OP_POP);
    compile_with_precedence(this, PRECEDENCE_OR);
    patch_jump(this, jump);
}

static Function *end_function(Compiler *this) {
    emit_two(this, OP_NIL, OP_RETURN);
    Function *func = this->scope->func;
    this->scope = this->scope->enclosing;
    return func;
}

static void compile_function(Compiler *this, enum FunctionType type) {
    Scope scope = {0};
    compiler_scope_init(this, &scope, type);

    begin_scope(this);

    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            this->scope->func->arity++;
            if (this->scope->func->arity > 255) {
                compile_error(this, &this->alpha, "Can't have more than 255 function parameters.");
            }
            u8 parameter = variable(this, false, "Expected parameter name.");
            define_variable(this, parameter);
        } while (match(this, TOKEN_COMMA));
    }

    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after function parameters.");

    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }

    end_scope(this);
    consume(this, TOKEN_END, "Expected 'end' after function body.");

    Function *func = end_function(this);

    write_constant(current(this), new_func(func), this->alpha.row);
}

static void declare_function(Compiler *this) {
    u8 global = variable(this, false, "Expected function name.");
    local_initialize(this);
    compile_function(this, TYPE_FUNCTION);
    define_variable(this, global);
}

static void declaration(Compiler *this) {
    if (match(this, TOKEN_LET)) {
        declare_new_variable(this, false);
    } else if (match(this, TOKEN_CONST)) {
        declare_new_variable(this, true);
    } else if (match(this, TOKEN_FUNCTION)) {
        declare_function(this);
    } else {
        statement(this);
    }
}

static void block(Compiler *this) {
    begin_scope(this);
    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);
}

static void if_statement(Compiler *this) {
    expression(this);
    int jump = jump_instruction(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);

    begin_scope(this);
    while (!check(this, TOKEN_END) && !check(this, TOKEN_ELSE) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    int jump_else = jump_instruction(this, OP_JUMP);
    patch_jump(this, jump);
    emit(this, OP_POP);
    if (match(this, TOKEN_ELSE)) {
        block(this);
    }
    patch_jump(this, jump_else);
    consume(this, TOKEN_END, "Expected 'end' after if statement.");
}

static void emit_loop(Compiler *this, int start) {
    emit(this, OP_LOOP);
    int offset = current(this)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->alpha, "Loop is too large.");
    }
    emit_two(this, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX);
}

static void for_statement(Compiler *this) {
    begin_scope(this);

    if (match(this, TOKEN_LET)) {
        declare_new_variable(this, false);
    } else if (match(this, TOKEN_CONST)) {
        declare_new_variable(this, true);
    } else if (!check(this, TOKEN_SEMICOLON)) {
        expression_statement(this);
    }

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    int start = current(this)->count;
    int jump = -1;

    if (!check(this, TOKEN_SEMICOLON)) {
        expression(this);

        jump = jump_instruction(this, OP_JUMP_IF_FALSE);
        emit(this, OP_POP);
    }

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    int body = jump_instruction(this, OP_JUMP);
    int increment = current(this)->count;

    expression(this);

    emit(this, OP_POP);
    emit_loop(this, start);
    start = increment;
    patch_jump(this, body);

    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }

    emit_loop(this, start);

    if (jump != -1) {
        patch_jump(this, jump);
        emit(this, OP_POP);
    }

    end_scope(this);
    consume(this, TOKEN_END, "Expected 'end' after for loop.");
}

static void while_statement(Compiler *this) {
    int start = current(this)->count;
    expression(this);
    int jump = jump_instruction(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);
    block(this);
    emit_loop(this, start);
    patch_jump(this, jump);
    emit(this, OP_POP);
    consume(this, TOKEN_END, "Expected 'end' after while loop.");
}

static void return_statement(Compiler *this) {
    if (this->scope->type == TYPE_SCRIPT) {
        compile_error(this, &this->alpha, "Can't return from outside a function.");
    }
    expression(this);
    emit(this, OP_RETURN);
}

static void statement(Compiler *this) {
    if (match(this, TOKEN_PRINT)) {
        print_statement(this);
    } else if (match(this, TOKEN_IF)) {
        if_statement(this);
    } else if (match(this, TOKEN_FOR)) {
        for_statement(this);
    } else if (match(this, TOKEN_WHILE)) {
        while_statement(this);
    } else if (match(this, TOKEN_RETURN)) {
        return_statement(this);
    } else if (match(this, TOKEN_BEGIN)) {
        block(this);
        consume(this, TOKEN_END, "Expected 'end' after block.");
    } else {
        expression_statement(this);
    }
    if (this->panic) {
        panic_halt(this);
    }
}

static void print_statement(Compiler *this) {
    expression(this);
    emit(this, OP_PRINT);
}

static void expression_statement(Compiler *this) {
    expression(this);
    emit(this, OP_POP);
}

static void expression(Compiler *this) {
    compile_with_precedence(this, PRECEDENCE_ASSIGN);
}

static Function *compile(Machine *machine, char *source, char **error) {

    Scope scope = {0};

    Compiler c = new_compiler(source, machine, &scope);
    Compiler *compiler = &c;

    advance(compiler);
    advance(compiler);
    while (!match(compiler, TOKEN_EOF)) {
        declaration(compiler);
    }

    if (compiler->error) {
        *error = string_to_chars(compiler->error);
    }

    Function *func = end_function(compiler);

    compiler_delete(compiler);

    return func;
}

#ifdef HYMN_DEBUG_TRACE
static usize debug_constant_instruction(const char *name, ByteCode *this, usize index) {
    u8 constant = this->instructions[index + 1];
    printf("%s: [%d: ", name, constant);
    debug_value(this->constants.values[constant]);
    printf("]\n");
    return index + 2;
}

static usize debug_byte_instruction(const char *name, ByteCode *this, usize index) {
    u8 b = this->instructions[index + 1];
    printf("%s: [%d]\n", name, b);
    return index + 2;
}

static usize debug_jump_instruction(const char *name, int sign, ByteCode *this, usize index) {
    u16 jump = (u16)(this->instructions[index + 1] << 8) | (u16)this->instructions[index + 2];
    printf("%s: [%zu] -> [%zu]\n", name, index, index + 3 + sign * jump);
    return index + 3;
}

static usize debug_instruction(const char *name, usize index) {
    printf("%s\n", name);
    return index + 1;
}

static usize disassemble_instruction(ByteCode *this, usize index) {
    printf("%04zu ", index);
    if (index > 0 and this->rows[index] == this->rows[index - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", this->rows[index]);
    }
    u8 op = this->instructions[index];
    switch (op) {
    case OP_RETURN: return debug_instruction("OP_RETURN", index);
    case OP_ADD: return debug_instruction("OP_ADD", index);
    case OP_SUBTRACT: return debug_instruction("OP_SUBTRACT", index);
    case OP_MULTIPLY: return debug_instruction("OP_MULTIPLY", index);
    case OP_DIVIDE: return debug_instruction("OP_DIVIDE", index);
    case OP_NEGATE: return debug_instruction("OP_NEGATE", index);
    case OP_TRUE: return debug_instruction("OP_TRUE", index);
    case OP_FALSE: return debug_instruction("OP_FALSE", index);
    case OP_NIL: return debug_instruction("OP_NIL", index);
    case OP_NOT: return debug_instruction("OP_NOT", index);
    case OP_EQUAL: return debug_instruction("OP_EQUAL", index);
    case OP_NOT_EQUAL: return debug_instruction("OP_NOT_EQUAL", index);
    case OP_GREATER: return debug_instruction("OP_GREATER", index);
    case OP_GREATER_EQUAL: return debug_instruction("OP_GREATER_EQUAL", index);
    case OP_LESS: return debug_instruction("OP_LESS", index);
    case OP_LESS_EQUAL: return debug_instruction("OP_LESS_EQUAL", index);
    case OP_PRINT: return debug_instruction("OP_PRINT", index);
    case OP_POP: return debug_instruction("OP_POP", index);
    case OP_LOOP: return debug_jump_instruction("OP_LOOP", -1, this, index);
    case OP_JUMP: return debug_jump_instruction("OP_JUMP", 1, this, index);
    case OP_JUMP_IF_FALSE: return debug_jump_instruction("OP_JUMP_IF_FALSE", 1, this, index);
    case OP_CONSTANT: return debug_constant_instruction("OP_CONSTANT", this, index);
    case OP_DEFINE_GLOBAL: return debug_constant_instruction("OP_DEFINE_GLOBAL", this, index);
    case OP_SET_GLOBAL: return debug_constant_instruction("OP_SET_GLOBAL", this, index);
    case OP_GET_GLOBAL: return debug_constant_instruction("OP_GET_GLOBAL", this, index);
    case OP_SET_LOCAL: return debug_byte_instruction("OP_SET_LOCAL", this, index);
    case OP_GET_LOCAL: return debug_byte_instruction("OP_GET_LOCAL", this, index);
    case OP_CALL: return debug_byte_instruction("OP_CALL", this, index);
    default: printf("UNKNOWN OPCODE %d\n", op); return index + 1;
    }
}
#endif

static void machine_reset_stack(Machine *this) {
    this->stack_top = 0;
    this->frame_count = 0;
}

static void machine_runtime_error(Machine *this, const char *format, ...) {
    if (this->error == NULL) {
        this->error = new_string("");
    }

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    this->error = string_append(this->error, chars);
    free(chars);

    this->error = string_append_char(this->error, '\n');

    for (int i = this->frame_count - 1; i >= 0; i--) {
        Frame *frame = &this->frames[i];
        Function *func = frame->func;
        usize ip = frame->ip - 1;
        int row = frame->func->code.rows[ip];

        this->error = string_append_format(this->error, "[Line %d] in ", row);
        if (func->name == NULL) {
            this->error = string_append_format(this->error, "script\n");
        } else {
            this->error = string_append_format(this->error, "%s()\n", func->name);
        }
    }

    machine_reset_stack(this);
}

static void machine_push(Machine *this, Value value) {
    this->stack[this->stack_top++] = value;
}

static Value machine_peek(Machine *this, int dist) {
    if (dist > this->stack_top) {
        machine_runtime_error(this, "Nothing on stack to peek");
        return new_nil();
    }
    return this->stack[this->stack_top - dist];
}

static Value machine_pop(Machine *this) {
    if (this->stack_top == 0) {
        machine_runtime_error(this, "Nothing on stack to pop");
        return new_nil();
    }
    return this->stack[--this->stack_top];
}

static bool machine_equal(Value a, Value b) {
    switch (a.is) {
    case VALUE_NIL: return is_nil(b);
    case VALUE_BOOL: return is_bool(b) ? as_bool(a) == as_bool(b) : false;
    case VALUE_INTEGER:
        switch (b.is) {
        case VALUE_INTEGER: return as_int(a) == as_int(b);
        case VALUE_FLOAT: return (double)as_int(a) == as_float(b);
        default: return false;
        }
    case VALUE_FLOAT:
        switch (b.is) {
        case VALUE_INTEGER: return as_float(a) == (double)as_int(b);
        case VALUE_FLOAT: return as_float(a) == as_float(b);
        default: return false;
        }
    case VALUE_STRING:
        switch (b.is) {
        case VALUE_STRING: return as_string(a) == as_string(b);
        default: return false;
        }
    case VALUE_FUNC:
        switch (b.is) {
        case VALUE_FUNC: return as_func(a) == as_func(b);
        default: return false;
        }
    case VALUE_FUNC_NATIVE:
        switch (b.is) {
        case VALUE_FUNC_NATIVE: return as_native(a) == as_native(b);
        default: return false;
        }
    default: return false;
    }
}

static bool machine_false(Value value) {
    switch (value.is) {
    case VALUE_NIL: return true;
    case VALUE_BOOL: return !as_bool(value);
    case VALUE_INTEGER: return as_int(value) == 0;
    case VALUE_FLOAT: return as_float(value) == 0.0;
    case VALUE_STRING: return string_len(as_string(value)) == 0;
    case VALUE_FUNC: return as_func(value) == NULL;
    case VALUE_FUNC_NATIVE: return as_native(value) == NULL;
    default: return false;
    }
}

static bool machine_call(Machine *this, Function *func, int count) {
    if (count != func->arity) {
        machine_runtime_error(this, "Expected %d function arguments but found %d.", func->arity, count);
        return false;
    }

    if (this->frame_count == HYMN_FRAMES_MAX) {
        machine_runtime_error(this, "Stack overflow.");
        return false;
    }

    Frame *frame = &this->frames[this->frame_count++];
    frame->func = func;
    frame->ip = 0;
    frame->stack_top = this->stack_top - count - 1;

    return true;
}

static bool machine_call_value(Machine *this, Value call, int count) {
    switch (call.is) {
    case VALUE_FUNC:
        return machine_call(this, as_func(call), count);
    case VALUE_FUNC_NATIVE: {
        NativeCall native = as_native(call)->native;
        Value result = native(count, &this->stack[this->stack_top - count]);
        this->stack_top -= count + 1;
        machine_push(this, result);
        return true;
    }
    default:
        machine_runtime_error(this, "Only functions can be called.");
        return false;
    }
}

static inline u8 read_byte(Frame *frame) {
    return frame->func->code.instructions[frame->ip++];
}

static inline u16 read_short(Frame *frame) {
    frame->ip += 2;
    return ((u16)frame->func->code.instructions[frame->ip - 2] << 8) | (u16)frame->func->code.instructions[frame->ip - 1];
}

static inline Value read_constant(Frame *frame) {
    return frame->func->code.constants.values[read_byte(frame)];
}

static void machine_run(Machine *this) {
    Frame *frame = &this->frames[this->frame_count - 1];
    while (true) {
#ifdef HYMN_DEBUG_STACK
        if (this->stack_top > 0) {
            printf("STACK ================================================== ");
            for (usize i = 0; i < this->stack_top; i++) {
                printf("[%zu: ", i);
                debug_value(this->stack[i]);
                printf("] ");
            }
            printf("\n");
        }
#endif
#ifdef HYMN_DEBUG_TRACE
        disassemble_instruction(&frame->func->code, frame->ip);
#endif
        u8 op = read_byte(frame);
        switch (op) {
        case OP_RETURN:
            Value result = machine_pop(this);
            this->frame_count--;
            if (this->frame_count == 0) {
                machine_pop(this);
                return;
            }
            this->stack_top = frame->stack_top;
            machine_push(this, result);
            frame = &this->frames[this->frame_count - 1];
            break;
        case OP_POP:
            machine_pop(this);
            break;
        case OP_TRUE:
            machine_push(this, new_bool(true));
            break;
        case OP_FALSE:
            machine_push(this, new_bool(false));
            break;
        case OP_NIL:
            machine_push(this, new_nil());
            break;
        case OP_CALL: {
            int count = read_byte(frame);
            Value peek = machine_peek(this, count + 1);
            if (!machine_call_value(this, peek, count)) {
                return;
            }
            frame = &this->frames[this->frame_count - 1];
            break;
        }
        case OP_JUMP: {
            u16 jump = read_short(frame);
            frame->ip += jump;
            break;
        }
        case OP_JUMP_IF_FALSE: {
            u16 jump = read_short(frame);
            if (machine_false(machine_peek(this, 1))) {
                frame->ip += jump;
            }
            break;
        }
        case OP_LOOP: {
            u16 jump = read_short(frame);
            frame->ip -= jump;
            break;
        }
        case OP_EQUAL: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            machine_push(this, new_bool(machine_equal(a, b)));
            break;
        }
        case OP_NOT_EQUAL: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            machine_push(this, new_bool(!machine_equal(a, b)));
            break;
        }
        case OP_LESS: {
            macro_compare_op(<);
            break;
        }
        case OP_LESS_EQUAL: {
            macro_compare_op(<=);
            break;
        }
        case OP_GREATER: {
            macro_compare_op(>);
            break;
        }
        case OP_GREATER_EQUAL: {
            macro_compare_op(>=);
            break;
        }
        case OP_ADD: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (is_nil(a)) {
                if (is_string(b)) {
                    String *temp = new_string(STR_NIL);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_bool(a)) {
                if (is_string(b)) {
                    String *temp = new_string(as_bool(a) ? STR_TRUE : STR_FALSE);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_int(a)) {
                if (is_int(b)) {
                    a.as.i += b.as.i;
                    machine_push(this, a);
                } else if (is_float(b)) {
                    b.as.f += a.as.i;
                    machine_push(this, a);
                } else if (is_string(b)) {
                    String *temp = int64_to_string(as_int(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_float(a)) {
                if (is_int(b)) {
                    a.as.f += b.as.i;
                    machine_push(this, a);
                } else if (is_float(b)) {
                    a.as.f += b.as.f;
                    machine_push(this, a);
                } else if (is_string(b)) {
                    String *temp = float64_to_string(as_float(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_string(a)) {
                String *s = as_string(a);
                String *add = NULL;
                switch (b.is) {
                case VALUE_NIL:
                    add = string_append(string_copy(s), "Nil");
                    break;
                case VALUE_BOOL:
                    add = string_append(string_copy(s), as_bool(b) ? STR_TRUE : STR_FALSE);
                    break;
                case VALUE_INTEGER: {
                    String *temp = int64_to_string(as_int(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_FLOAT: {
                    String *temp = float64_to_string(as_float(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_STRING:
                    add = string_concat(s, as_string(b));
                    break;
                case VALUE_FUNC:
                    add = string_concat(s, as_func(b)->name);
                    break;
                case VALUE_FUNC_NATIVE:
                    add = string_concat(s, as_native(b)->name);
                    break;
                default:
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
                machine_push(this, machine_intern_string(this, add));
            } else {
                machine_runtime_error(this, "Operands can't be added.");
                return;
            }
            break;
        }
        case OP_SUBTRACT: {
            macro_arithmetic_op(-=);
            break;
        }
        case OP_MULTIPLY: {
            macro_arithmetic_op(*=);
            break;
        }
        case OP_DIVIDE: {
            macro_arithmetic_op(/=);
            break;
        }
        case OP_NEGATE: {
            Value value = machine_peek(this, 1);
            if (is_int(value)) {
                value.as.i = -value.as.i;
            } else if (is_float(value)) {
                value.as.f = -value.as.f;
            } else {
                machine_runtime_error(this, "Operand must be a number.");
                return;
            }
            machine_pop(this);
            machine_push(this, value);
            break;
        }

        case OP_NOT: {
            Value value = machine_peek(this, 1);
            if (is_bool(value)) {
                value.as.b = !value.as.b;
            } else {
                machine_runtime_error(this, "Operand must be a boolean.");
                return;
            }
            machine_pop(this);
            machine_push(this, value);
            break;
        }
        case OP_CONSTANT: {
            Value constant = read_constant(frame);
            machine_push(this, constant);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value set = machine_peek(this, 1);
            map_put(&this->globals, name, set);
            machine_pop(this);
            break;
        }
        case OP_SET_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value set = machine_peek(this, 1);
            Value exists = map_get(&this->globals, name);
            if (is_undefined(exists)) {
                machine_runtime_error(this, "Undefined variable '%s'.", name);
                return;
            }
            map_put(&this->globals, name, set);
            break;
        }
        case OP_GET_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value get = map_get(&this->globals, name);
            if (is_undefined(get)) {
                machine_runtime_error(this, "Undefined variable '%s'.", name);
                return;
            }
            machine_push(this, get);
            break;
        }
        case OP_SET_LOCAL: {
            u8 slot = read_byte(frame);
            this->stack[slot] = machine_peek(this, 1);
            break;
        }
        case OP_GET_LOCAL: {
            u8 slot = read_byte(frame);
            machine_push(this, this->stack[slot]);
            break;
        }
        case OP_PRINT:
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_NIL:
                printf("%s\n", STR_NIL);
                break;
            case VALUE_BOOL:
                printf("%s\n", as_bool(value) ? STR_TRUE : STR_FALSE);
                break;
            case VALUE_INTEGER:
                printf("%" PRId64 "\n", as_int(value));
                break;
            case VALUE_FLOAT:
                printf("%g\n", as_float(value));
                break;
            case VALUE_STRING:
                printf("%s\n", as_string(value));
                break;
            case VALUE_FUNC:
                printf("%s\n", as_func(value)->name);
                break;
            case VALUE_FUNC_NATIVE:
                printf("%s\n", as_func(value)->name);
                break;
            default:
                printf("%p\n", &value);
            }
            break;
        default:
            machine_runtime_error(this, "Unknown instruction.");
            return;
        }
    }
}

static char *machine_interpret(Machine *this) {
    machine_run(this);

    char *error = NULL;
    if (this->error) {
        error = string_copy(this->error);
    }
    return error;
}

static void add_func(Machine *this, char *name, NativeCall func) {
    Value intern = machine_intern_string(this, new_string(name));
    NativeFunction *value = new_native_function(as_string(intern), func);
    map_put(&this->globals, as_string(intern), new_native(value));
}

static inline Machine new_machine() {
    Machine this = {0};
    machine_reset_stack(&this);
    map_init(&this.strings);
    map_init(&this.globals);
    return this;
}

static void machine_delete(Machine *this) {
    map_delete(&this->strings);
    map_delete(&this->globals);
    string_delete(this->error);
}

Hymn *new_hymn() {
    Hymn *this = safe_calloc(1, sizeof(Hymn));
    this->scripts = new_string_table();
    return this;
}

static Value temp_native_test(int count, Value *arguments) {
    if (count == 0) {
        return new_nil();
    }
    i64 i = as_int(arguments[0]) + 1;
    return new_int(i);
}

char *hymn_eval(Hymn *this, char *source) {
    (void *)this;

    Machine m = new_machine();
    Machine *machine = &m;

    add_func(machine, "inc", temp_native_test);

    char *error = NULL;

    Function *func = compile(machine, source, &error);
    if (error) {
        return error;
    }

    machine_push(machine, new_func(func));
    machine_call(machine, func, 0);

    error = machine_interpret(machine);
    if (error) {
        return error;
    }

    machine_reset_stack(machine);

    machine_delete(machine);
    return error;
}

char *hymn_read(Hymn *this, char *file) {
    String *source = cat(file);
    char *error = hymn_eval(this, source);
    string_delete(source);
    return error;
}

char *hymn_repl(Hymn *this) {
    (void *)this;
    printf("Welcome to Hymn\n");

    char input[1024];
    char *error = NULL;

    Machine m = new_machine();
    Machine *machine = &m;

    while (true) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        Function *func = compile(machine, input, &error);
        if (error) {
            break;
        }

        machine_push(machine, new_func(func));
        machine_call(machine, func, 0);

        error = machine_interpret(machine);
        if (error) {
            return error;
        }

        machine_reset_stack(machine);
    }

    machine_delete(machine);
    return error;
}

void hymn_add_func(Hymn *this, char *name, char *(*func)(Hymn *)) {
    (void *)this;
    (void *)name;
    (void *)func;
}

void hymn_add_pointer(Hymn *this, char *name, void *pointer) {
    (void *)this;
    (void *)name;
    (void *)pointer;
}

char *hymn_call(Hymn *this, char *name) {
    (void *)this;
    (void *)name;
    return NULL;
}

void *hymn_pointer(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return NULL;
}

i32 hymn_i32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

u32 hymn_u32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

i64 hymn_i64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

u64 hymn_u64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

f32 hymn_f32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0.0f;
}

f64 hymn_f64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0.0;
}

void hymn_delete(Hymn *this) {
    table_delete(this->scripts);
    free(this);
}
