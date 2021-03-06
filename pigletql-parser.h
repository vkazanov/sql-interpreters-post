#ifndef PIGLETQL_PARSER_H
#define PIGLETQL_PARSER_H

#include <stdbool.h>

#include "pigletql-def.h"

typedef enum token_type {
    TOKEN_IDENT,
    TOKEN_NUMBER,

    TOKEN_STAR,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_EQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,

    TOKEN_SELECT,
    TOKEN_CREATE,
    TOKEN_TABLE,
    TOKEN_INSERT,

    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_AND,

    TOKEN_ORDER,
    TOKEN_BY,
    TOKEN_ASC,
    TOKEN_DESC,

    TOKEN_INTO,
    TOKEN_VALUES,

    TOKEN_ERROR,                /* failed to scan */
    TOKEN_EOS                   /* end of stream */
} token_type;

typedef struct token_t {
    token_type type;            /* token type tag */
    const char *start;          /* start of the token */
    int length;                 /* length of the token string */
} token_t;

typedef struct query_predicate_t {
    token_t left;
    token_t op;
    token_t right;
} query_predicate_t;

typedef enum query_tag {
    QUERY_SELECT,
    QUERY_CREATE_TABLE,
    QUERY_INSERT,
} query_tag;

typedef struct query_select_t {
    /* Attributes to output */
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;

    /* Relations to get tuples from */
    rel_name_t rel_names[MAX_REL_NUM];
    uint16_t rel_num;

    /* Predicates to apply to tuples */
    query_predicate_t predicates[MAX_PRED_NUM];
    uint16_t pred_num;

    /* Pick an attribute to sort by */
    bool has_order;
    attr_name_t order_by_attr;
    sort_order_t order_type;
} query_select_t;

typedef struct query_create_table_t {
    rel_name_t rel_name;

    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;
} query_create_table_t;

typedef struct query_insert_t {
    rel_name_t rel_name;

    value_type_t values[MAX_ATTR_NUM];
    uint16_t value_num;
} query_insert_t;

typedef struct query_t {
    query_tag tag;
    union {
        query_select_t select;
        query_create_table_t create_table;
        query_insert_t insert;
    } as;
} query_t;

typedef struct parser_t parser_t;

typedef struct scanner_t scanner_t;

scanner_t *scanner_create(const char *string);

void scanner_destroy(scanner_t *scanner);

token_t scanner_next(scanner_t *scanner);

query_t *query_create(void);

void query_destroy(query_t *query);

parser_t *parser_create(void);

void parser_destroy(parser_t *parser);

bool parser_parse(parser_t *parser, scanner_t *scanner, query_t *query);

#endif //PIGLETQL_PARSER_H
