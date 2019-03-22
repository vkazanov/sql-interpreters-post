#ifndef PIGLETQL_PARSER_H
#define PIGLETQL_PARSER_H

typedef struct query_t query_t;

typedef struct scanner_t scanner_t;

typedef enum token_type {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    TOKEN_COMMA,
    TOKEN_SEMICOLON,

    TOKEN_SELECT,
    TOKEN_INSERT,
    TOKEN_CREATE,

    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_ORDER_BY,
    TOKEN_ASC,
    TOKEN_DESC,
} token_type;

typedef struct token {
    token_type type;
    const char *start;
    int length;
} token;

scanner_t *scanner_create(const char *string);

void scanner_destroy(scanner_t *scanner);

void parse(const char *query_string, query_t *query);

#endif //PIGLETQL_PARSER_H
