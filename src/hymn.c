/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

enum VarType {
    TYPE_BOOL,
    TYPE_FLOAT,
    TYPE_FUNCTION,
    TYPE_INT,
    TYPE_LIST,
    TYPE_NIL,
    TYPE_STRING,
    TYPE_TABLE,
};

enum TokenType {
    TOKEN_AND,
    TOKEN_ASSIGN,
    TOKEN_COMMA,
    TOKEN_COMMENT,
    TOKEN_CONST,
    TOKEN_DIVIDE,
    TOKEN_DOT,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_END,
    TOKEN_EOF,
    TOKEN_EQ,
    TOKEN_ERROR,
    TOKEN_FLOAT,
    TOKEN_FOR,
    TOKEN_FUNC,
    TOKEN_GT,
    TOKEN_GTEQ,
    TOKEN_IDENT,
    TOKEN_IF,
    TOKEN_IMPORT,
    TOKEN_INT,
    TOKEN_LBRACE,
    TOKEN_LBRACKET,
    TOKEN_LINE,
    TOKEN_LPAREN,
    TOKEN_LT,
    TOKEN_LTEQ,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SUBTRACT,
    TOKEN_MULTIPLY,
    TOKEN_NIL,
    TOKEN_NOT,
    TOKEN_NOTEQ,
    TOKEN_NUMBER,
    TOKEN_OR,
    TOKEN_PASS,
    TOKEN_ADD,
    TOKEN_RBRACE,
    TOKEN_RBRACKET,
    TOKEN_RPAREN,
    TOKEN_STRING,
    TOKEN_UNDEFINED,
    TOKEN_VALUE,
    TOKEN_LET,
    TOKEN_WHILE,
    TOKEN_RETURN,
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
    OP_MULTIPLY,
    OP_NEGATE,
    OP_SUBTRACT,
    OP_RETURN,
};

typedef struct Keyword Keyword;
typedef struct Token Token;
typedef struct Rule Rule;
typedef struct Function Function;
typedef struct Value Value;
typedef struct Script Script;
typedef struct Compiler Compiler;
typedef struct ValuePool ValuePool;
typedef struct ByteCode ByteCode;
typedef struct Machine Machine;

struct Function {
    const char *name;
    int arity;
};

struct Value {
    enum VarType type;
    union {
        bool b;
        i64 i;
        double f;
        String *str;
        Function *func;
    } value;
};

struct Token {
    int row;
    int column;
    enum TokenType type;
    usize start;
    usize end;
    Value value;
};

static void parse_with_precedence(Compiler *compiler, ByteCode *code, enum Precedence precedence);
static void expression(Compiler *compiler, ByteCode *code);
static void unary(Compiler *compiler, ByteCode *code);
static void binary(Compiler *compiler, ByteCode *code);

struct Rule {
    void (*prefix)(Compiler *, ByteCode *);
    void (*infix)(Compiler *, ByteCode *);
    enum Precedence precedence;
};

Rule rules[] = {
    [TOKEN_ADD] = {NULL, binary, PRECEDENCE_TERM},
    [TOKEN_AND] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMENT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CONST] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_DIVIDE] = {NULL, binary, PRECEDENCE_FACTOR},
    [TOKEN_DOT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELIF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_END] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EOF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EQ] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FALSE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FLOAT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FOR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FUNC] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_GT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_GTEQ] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_IDENT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_IMPORT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_INT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LBRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LBRACKET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LINE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LPAREN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LTEQ] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_MULTIPLY] = {NULL, binary, PRECEDENCE_FACTOR},
    [TOKEN_NIL] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_NOT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_NOTEQ] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_NUMBER] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_OR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_PASS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RBRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RBRACKET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RPAREN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_STRING] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_SUBTRACT] = {unary, binary, PRECEDENCE_TERM},
    [TOKEN_TRUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EQ] = {NULL, NULL, PRECEDENCE_NONE},
};

struct Script {
    const char *name;
    Value **variables;
    usize variable_count;
};

struct Compiler {
    usize pos;
    int row;
    int column;
    char *source;
    usize size;
    String *temp;
    Token alpha;
    Token beta;
    Token gamma;
};

struct ValuePool {
    int count;
    int capacity;
    Value *values;
};

struct ByteCode {
    int count;
    int capacity;
    u8 *code;
    int *rows;
    ValuePool constants;
};

struct Machine {
    ByteCode *code;
    usize ip;
    Value stack[HYMN_STACK_MAX];
    usize stack_top;
};

static const char *value_name(enum VarType value) {
    switch (value) {
    case TYPE_BOOL: return "BOOL";
    case TYPE_INT: return "INT";
    case TYPE_FLOAT: return "FLOAT";
    case TYPE_STRING: return "STRING";
    default: return "UNKNOWN";
    }
}

static const char *token_name(enum TokenType token) {
    switch (token) {
    case TOKEN_IMPORT: return "IMPORT";
    case TOKEN_FOR: return "FOR";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_AND: return "AND";
    case TOKEN_OR: return "OR";
    case TOKEN_FLOAT: return "FLOAT";
    case TOKEN_INT: return "INT";
    case TOKEN_LPAREN: return "LPAREN";
    case TOKEN_RPAREN: return "RPAREN";
    case TOKEN_ADD: return "ADD";
    case TOKEN_IDENT: return "IDENT";
    case TOKEN_FUNC: return "FUNC";
    case TOKEN_EOF: return "EOF";
    case TOKEN_IF: return "IF";
    default: return "?";
    }
}

static void debug_value(Value value) {
    printf("%s: ", value_name(value.type));
    switch (value.type) {
    case TYPE_INT: printf("%" PRId64, value.value.i); break;
    case TYPE_FLOAT: printf("%g", value.value.f); break;
    default: printf("?");
    }
}

static inline Compiler new_compiler(char *source) {
    Compiler compiler = {0};
    compiler.pos = 0;
    compiler.row = 1;
    compiler.column = 1;
    compiler.source = source;
    compiler.size = strlen(source);
    compiler.temp = new_string("");
    compiler.alpha.type = TOKEN_UNDEFINED;
    compiler.beta.type = TOKEN_UNDEFINED;
    compiler.gamma.type = TOKEN_UNDEFINED;
    return compiler;
}

static void compiler_delete(Compiler *this) {
    string_delete(this->temp);
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

static void push_token(Compiler *this, enum TokenType type) {
    printf("TOKEN: %s\n", token_name(type));
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = type;
}

static void push_value_token(Compiler *this, enum TokenType type, usize start, usize end, Value value) {
    printf("TOKEN: %s [", token_name(type));
    debug_value(value);
    printf("], %.*s\n", (int)(end - start), &this->source[start]);
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = type;
    gamma->start = start;
    gamma->end = end;
    gamma->value = value;
}

static void push_string_token(Compiler *this, usize start, usize end) {
    printf("TOKEN: %s, %.*s\n", token_name(TOKEN_STRING), (int)(end - start), &this->source[start]);
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = TOKEN_STRING;
    gamma->start = start;
    gamma->end = end;
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

#define keyword_size(a, b)      \
    if (a != b)                 \
        return TOKEN_UNDEFINED; \
    else                        \
        return

static enum TokenType ident_keyword(char *ident, usize size) {
    switch (ident[0]) {
    case 'o': keyword_size(size, 2) ident_trie(ident, 1, "r", TOKEN_OR);
    case 'a': keyword_size(size, 3) ident_trie(ident, 1, "nd", TOKEN_AND);
    case 'e': keyword_size(size, 3) ident_trie(ident, 1, "nd", TOKEN_END);
    case 'n': keyword_size(size, 3) ident_trie(ident, 1, "il", TOKEN_NIL);
    case 'l': keyword_size(size, 3) ident_trie(ident, 1, "et", TOKEN_LET);
    case 'c': keyword_size(size, 5) ident_trie(ident, 1, "onst", TOKEN_CONST);
    case 'w': keyword_size(size, 5) ident_trie(ident, 1, "hile", TOKEN_WHILE);
    case 'r': keyword_size(size, 6) ident_trie(ident, 1, "eturn", TOKEN_RETURN);
    case 'i':
        if (size > 1) {
            switch (ident[1]) {
            case 'f': return TOKEN_IF;
            case 'm': keyword_size(size, 6) ident_trie(ident, 1, "port", TOKEN_IMPORT);
            }
        }
    case 'f':
        if (size > 1) {
            switch (ident[1]) {
            case 'o': keyword_size(size, 3) ident_trie(ident, 2, "r", TOKEN_FOR);
            case 'a': keyword_size(size, 5) ident_trie(ident, 2, "lse", TOKEN_FALSE);
            case 'u': keyword_size(size, 8) ident_trie(ident, 2, "nction", TOKEN_FUNC);
            }
            break;
        }
    }
    return TOKEN_UNDEFINED;
}

static void push_ident_token(Compiler *this, usize start, usize end) {
    char *ident = &this->source[start];
    usize size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        push_token(this, keyword);
        return;
    }
    printf("TOKEN: %s, %.*s\n", token_name(TOKEN_IDENT), (int)(end - start), &this->source[start]);
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = TOKEN_IDENT;
    gamma->start = start;
    gamma->end = end;
}

static bool is_digit(char c) {
    return '0' <= c and c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or c == '_';
}

static void advance_token(Compiler *this) {
    LOG("ADVANCE_TOKEN");
    this->alpha = this->beta;
    this->beta = this->gamma;

    char c = next_char(this);
    switch (c) {
    case '#':
        c = peek_char(this);
        while (c != '\n' and c != '\0') {
            c = next_char(this);
        }
        next_char(this);
        break;
    case ' ':
    case '\t':
    case '\r':
        c = peek_char(this);
        while (c != '\0' and (c == ' ' or c == '\t' or c == '\r')) {
            c = next_char(this);
        }
        break;
    case '!':
        if (peek_char(this) == '=') {
            next_char(this);
            push_token(this, TOKEN_NOTEQ);
        } else {
            push_token(this, TOKEN_NOT);
        }
        break;
    case '=':
        if (peek_char(this) == '=') {
            next_char(this);
            push_token(this, TOKEN_EQ);
        } else {
            push_token(this, TOKEN_ASSIGN);
        }
        break;
    case '>':
        if (peek_char(this) == '=') {
            next_char(this);
            push_token(this, TOKEN_GTEQ);
        } else {
            push_token(this, TOKEN_GT);
        }
        break;
    case '<':
        if (peek_char(this) == '=') {
            next_char(this);
            push_token(this, TOKEN_LTEQ);
        } else {
            push_token(this, TOKEN_LT);
        }
        break;
    case '+': push_token(this, TOKEN_ADD); break;
    case '-': push_token(this, TOKEN_SUBTRACT); break;
    case '*': push_token(this, TOKEN_MULTIPLY); break;
    case '/': push_token(this, TOKEN_DIVIDE); break;
    case ',': push_token(this, TOKEN_COMMA); break;
    case '.': push_token(this, TOKEN_DOT); break;
    case '(': push_token(this, TOKEN_LPAREN); break;
    case ')': push_token(this, TOKEN_RPAREN); break;
    case '[': push_token(this, TOKEN_LBRACKET); break;
    case ']': push_token(this, TOKEN_RBRACKET); break;
    case '{': push_token(this, TOKEN_LBRACE); break;
    case '}': push_token(this, TOKEN_RBRACE); break;
    case '\0': push_token(this, TOKEN_EOF); return;
    case '"': {
        usize start = this->pos - 1;
        while (true) {
            c = peek_char(this);
            if (c == '"' or c == '\0') {
                break;
            }
            next_char(this);
        }
        usize end = this->pos;
        push_string_token(this, start, end);
        break;
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
            string_zero(this->temp);
            this->temp = string_append_substring(this->temp, this->source, start, end);
            if (discrete) {
                Value value = {0};
                value.type = TYPE_INT;
                value.value.i = string_to_int64(this->temp);
                push_value_token(this, TOKEN_INT, start, end, value);
            } else {
                Value value = {0};
                value.type = TYPE_FLOAT;
                value.value.f = string_to_float64(this->temp);
                push_value_token(this, TOKEN_FLOAT, start, end, value);
            }
        } else if (is_ident(c)) {
            usize start = this->pos - 1;
            while (is_ident(peek_char(this))) {
                next_char(this);
            }
            usize end = this->pos;
            push_ident_token(this, start, end);
        }
        break;
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
    this->code = safe_malloc(8 * sizeof(u8));
    this->rows = safe_malloc(8 * sizeof(int));
    value_pool_init(&this->constants);
}

static void byte_code_delete(ByteCode *this) {
    free(this->code);
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
        this->code = safe_realloc(this->code, this->capacity * sizeof(u8));
        this->rows = safe_realloc(this->rows, this->capacity * sizeof(int));
    }
    this->code[count] = b;
    this->rows[count] = row;
    this->count = count + 1;
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static void parse_error(char *error) {
    fprintf(stderr, error);
    fprintf(stderr, "\n");
    exit(1);
}

static void parse_with_precedence(Compiler *compiler, ByteCode *code, enum Precedence precedence) {
    LOG("PARSE_WITH_PRECEDENCE");
    advance_token(compiler);
    printf("ALPHA: %s, BETA: %s, GAMMA: %s\n", token_name(compiler->alpha.type), token_name(compiler->beta.type), token_name(compiler->gamma.type));
    Rule *rule = token_rule(compiler->alpha.type);
    void (*prefix)(Compiler *, ByteCode *) = rule->prefix;
    if (prefix == NULL) {
        parse_error("FATAL: EXPECTED EXPRESSION");
        return;
    }
    prefix(compiler, code);
    while (precedence <= token_rule(compiler->beta.type)->precedence) {
        advance_token(compiler);
        token_rule(compiler->alpha.type)->infix(compiler, code);
    }
}

static void expression(Compiler *compiler, ByteCode *code) {
    LOG("EXPRESSION");
    printf("ALPHA: %s, BETA: %s, GAMMA: %s\n", token_name(compiler->alpha.type), token_name(compiler->beta.type), token_name(compiler->gamma.type));
    parse_with_precedence(compiler, code, PRECEDENCE_ASSIGN);
}

static void unary(Compiler *compiler, ByteCode *code) {
    LOG("UNARY");
    parse_with_precedence(compiler, code, PRECEDENCE_UNARY);
    int row = compiler->alpha.row;
    enum TokenType type = compiler->alpha.type;
    expression(compiler, code);
    switch (type) {
    case TOKEN_SUBTRACT: write_op(code, OP_NEGATE, row); break;
    }
}

static void binary(Compiler *compiler, ByteCode *code) {
    LOG("BINARY");
    int row = compiler->alpha.row;
    enum TokenType type = compiler->alpha.type;
    Rule *rule = token_rule(type);
    parse_with_precedence(compiler, code, (enum Precedence)(rule->precedence + 1));
    switch (type) {
    case TOKEN_ADD: write_op(code, OP_ADD, row); break;
    case TOKEN_SUBTRACT: write_op(code, OP_SUBTRACT, row); break;
    case TOKEN_MULTIPLY: write_op(code, OP_MULTIPLY, row); break;
    case TOKEN_DIVIDE: write_op(code, OP_DIVIDE, row); break;
    }
}

static char *compile(ByteCode *code, char *source) {
    (void *)code;
    char *error = NULL;
    Compiler c = new_compiler(source);
    Compiler *compiler = &c;

    advance_token(compiler);
    advance_token(compiler);
    expression(compiler, code);

    // while (true) {
    //     LOG("EVAL");
    //     advance_token(compiler);
    //     if (c.gamma.type == TOKEN_EOF) {
    //         break;
    //     }
    // }
    compiler_delete(compiler);
    return error;
}

static usize debug_constant_instruction(const char *name, ByteCode *this, usize index) {
    u8 constant = this->code[index + 1];
    printf("%s: [%d: ", name, constant);
    debug_value(this->constants.values[constant]);
    printf("]\n");
    return index + 2;
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
    u8 op = this->code[index];
    switch (op) {
    case OP_RETURN: return debug_instruction("OP_RETURN", index);
    case OP_ADD: return debug_instruction("OP_ADD", index);
    case OP_SUBTRACT: return debug_instruction("OP_SUBTRACT", index);
    case OP_MULTIPLY: return debug_instruction("OP_MULTIPLY", index);
    case OP_DIVIDE: return debug_instruction("OP_DIVIDE", index);
    case OP_NEGATE: return debug_instruction("OP_NEGATE", index);
    case OP_CONSTANT: return debug_constant_instruction("OP_CONSTANT", this, index);
    default: printf("UNKNOWN OPCODE %d\n", op); return index + 1;
    }
}

static void disassemble_byte_code(ByteCode *this, const char *name) {
    printf("== %s ==", name);
    usize index = 0;
    while (index < this->count) {
        index = disassemble_instruction(this, index);
    }
}

static void machine_push(Machine *this, Value value) {
    this->stack[this->stack_top++] = value;
}

static Value machine_pop(Machine *this) {
    if (this->stack_top == 0) {
        fprintf(stderr, "ERROR: EMPTY STACK POP\n");
        exit(1);
    }
    return this->stack[--this->stack_top];
}

static void machine_reset_stack(Machine *this) {
    this->stack_top = 0;
}

static char *machine_run(Machine *this) {
    u8 *code = this->code->code;
    while (true) {
#ifdef HYMN_DEBUG_STACK
        if (this->stack_top > 0) {
            printf("STACK   ");
            for (usize i = 0; i < this->stack_top; i++) {
                printf("[%zu: ", i);
                debug_value(this->stack[i]);
                printf("] ");
            }
            printf("\n");
        }
#endif
#ifdef HYMN_DEBUG_TRACE
        disassemble_instruction(this->code, this->ip);
#endif
        u8 op = code[this->ip++];
        switch (op) {
        case OP_RETURN: return NULL;
        case OP_ADD: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (a.type == TYPE_INT) {
                if (b.type == TYPE_INT) {
                    a.value.i += b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    b.value.f += a.value.i;
                    machine_push(this, b);
                } else {
                    return "RUNTIME ERROR: ADD INT AND NON NUMBER";
                }
            } else if (a.type == TYPE_FLOAT) {
                if (b.type == TYPE_INT) {
                    a.value.f += b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    a.value.f += b.value.f;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: ADD FLOAT AND NON NUMBER";
                }
            } else {
                return "RUNTIME ERROR: ADD NON NUMBER";
            }
            break;
        }
        case OP_SUBTRACT: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (a.type == TYPE_INT) {
                if (b.type == TYPE_INT) {
                    a.value.i -= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    b.value.f -= a.value.i;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: SUBTRACT INT AND NON NUMBER";
                }
            } else if (a.type == TYPE_FLOAT) {
                if (b.type == TYPE_INT) {
                    a.value.f -= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    a.value.f -= b.value.f;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: SUBTRACT FLOAT AND NON NUMBER";
                }
            } else {
                return "RUNTIME ERROR: SUBTRACT NON NUMBER";
            }
            break;
        }
        case OP_MULTIPLY: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (a.type == TYPE_INT) {
                if (b.type == TYPE_INT) {
                    a.value.i *= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    b.value.f *= a.value.i;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: MULTIPLY INT AND NON NUMBER";
                }
            } else if (a.type == TYPE_FLOAT) {
                if (b.type == TYPE_INT) {
                    a.value.f *= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    a.value.f *= b.value.f;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: MULTIPLY FLOAT AND NON NUMBER";
                }
            } else {
                return "RUNTIME ERROR: MULTIPLY NON NUMBER";
            }
            machine_push(this, a);
            break;
        }
        case OP_DIVIDE: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (a.type == TYPE_INT) {
                if (b.type == TYPE_INT) {
                    a.value.i /= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    b.value.f /= a.value.i;
                    machine_push(this, b);
                } else {
                    return "RUNTIME ERROR: DIVIDE INT AND NON NUMBER";
                }
            } else if (a.type == TYPE_FLOAT) {
                if (b.type == TYPE_INT) {
                    a.value.f /= b.value.i;
                    machine_push(this, a);
                } else if (b.type == TYPE_FLOAT) {
                    a.value.f /= b.value.f;
                    machine_push(this, a);
                } else {
                    return "RUNTIME ERROR: DIVIDE FLOAT AND NON NUMBER";
                }
            } else {
                return "RUNTIME ERROR: DIVIDE NON NUMBER";
            }
            machine_push(this, a);
            break;
        }
        case OP_NEGATE: {
            Value pop = machine_pop(this);
            if (pop.type == TYPE_INT) {
                pop.value.i = -pop.value.i;
            } else if (pop.type == TYPE_FLOAT) {
                pop.value.f = -pop.value.f;
            } else {
                return "RUNTIME ERROR: NEGATE NON NUMBER";
            }
            machine_push(this, pop);
            break;
        }
        case OP_CONSTANT: {
            Value constant = this->code->constants.values[code[this->ip++]];
            machine_push(this, constant);
            break;
        }
        default: return "UNKNOWN OPCODE";
        }
    }
}

static char *machine_interpret(Machine *this, ByteCode *code) {
    this->code = code;
    this->ip = 0;
    return machine_run(this);
}

static inline Machine new_machine() {
    Machine machine = {0};
    machine_reset_stack(&machine);
    return machine;
}

Hymn *new_hymn() {
    Hymn *hymn = safe_calloc(1, sizeof(Hymn));
    hymn->scripts = new_string_table();
    return hymn;
}

char *hymn_eval(Hymn *this, char *source) {
    (void *)this;

    char *error = NULL;

    Machine m = new_machine();
    Machine *machine = &m;

    ByteCode bytes = {0};
    ByteCode *b = &bytes;
    byte_code_init(b);

    error = compile(b, source);
    if (error != NULL) {
        return error;
    }
    return machine_interpret(machine, b);

    // printf("==============================\n");

    // Machine m = new_machine();
    // Machine *machine = &m;

    // ByteCode bytes = {0};
    // ByteCode *b = &bytes;
    // byte_code_init(b);

    // {
    //     Value value = {0};
    //     value.type = TYPE_INT;
    //     value.value.i = 3;
    //     u8 constant = (u8)byte_code_add_constant(b, value);
    //     write_op(b, (u8)OP_CONSTANT, 1);
    //     write_op(b, constant, 1);
    // }

    // write_op(b, (u8)OP_NEGATE, 1);

    // {
    //     Value value = {0};
    //     value.type = TYPE_FLOAT;
    //     value.value.f = 13.2;
    //     u8 constant = (u8)byte_code_add_constant(b, value);
    //     write_op(b, (u8)OP_CONSTANT, 1);
    //     write_op(b, constant, 1);
    // }

    // write_op(b, (u8)OP_ADD, 1);
    // write_op(b, (u8)OP_RETURN, 1);

    // error = machine_interpret(machine, b);

    // if (error != NULL) {
    //     printf("%s\n", error);
    //     error = NULL;
    // } else {
    //     printf("RETURN: [");
    //     debug_value(machine_pop(machine));
    //     printf("]\n");
    // }

    // return error;
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

    ByteCode bytes = {0};
    ByteCode *b = &bytes;
    byte_code_init(b);

    while (true) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        error = compile(b, input);
        if (error != NULL) {
            return error;
        }
        error = machine_interpret(machine, b);
        if (error != NULL) {
            return error;
        }
    }
    return NULL;
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
