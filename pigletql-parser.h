#ifndef PIGLETQL_PARSER_H
#define PIGLETQL_PARSER_H

#include "pigletql-def.h"

typedef struct query_t {
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;

    rel_name_t rel_names[MAX_REL_NUM];
    uint16_t rel_num;
} query_t;


typedef struct parser_t parser_t;

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

parser_t *parser_create(void);

void parser_destroy(parser_t *parser);

void parser_parse(parser_t *parser, scanner_t *scanner, query_t *query);



#endif //PIGLETQL_PARSER_H
