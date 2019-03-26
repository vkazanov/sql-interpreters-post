#ifndef PIGLETQL_PARSER_H
#define PIGLETQL_PARSER_H

typedef struct query_t query_t;

typedef struct scanner_t scanner_t;

typedef enum token_type {
    TOKEN_IDENT,
    TOKEN_NUMBER,

    TOKEN_STAR,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_EQUAL,

    TOKEN_SELECT,
    /* TOKEN_INSERT, */
    /* TOKEN_CREATE, */

    TOKEN_FROM,
    TOKEN_WHERE,
    /* TOKEN_ORDER_BY, */
    /* TOKEN_ASC, */
    /* TOKEN_DESC, */

    TOKEN_ERROR,
    TOKEN_EOS
} token_type;

typedef struct token_t {
    token_type type;
    const char *start;
    int length;
} token_t;

scanner_t *scanner_create(const char *string);

void scanner_destroy(scanner_t *scanner);

token_t scanner_next(scanner_t *scanner);

query_t *query_create(void);

void query_destroy(query_t *query);

void query_parse(query_t *query, scanner_t *scanner);

#endif //PIGLETQL_PARSER_H
