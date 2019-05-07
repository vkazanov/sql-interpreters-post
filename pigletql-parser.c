#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pigletql-parser.h"

typedef struct scanner_t {
    const char *input;
    const char *token_start;
} scanner_t;

typedef struct parser_t {
    scanner_t *scanner;
    query_t *query;

    token_t current;
    token_t previous;

    bool had_error;
} parser_t;

scanner_t *scanner_create(const char *string)
{
    scanner_t *scanner = calloc(1, sizeof(*scanner));
    assert(scanner);

    scanner->input = string;

    return scanner;
}

void scanner_destroy(scanner_t *scanner)
{
    if (scanner)
        free(scanner);
}

static bool char_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || c == '_';
}

static bool char_is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static bool scanner_at_eos(scanner_t *scanner)
{
    return *scanner->input == '\0';
}

static char scanner_peek(scanner_t *scanner)
{
    return tolower(*scanner->input);
}

static char scanner_peek_start(scanner_t *scanner)
{
    return tolower(*scanner->token_start);
}

static char scanner_advance(scanner_t *scanner)
{
    return *scanner->input++;
}

static void scanner_skip_space(scanner_t *scanner)
{
    for (;;) {
        char c = scanner_peek(scanner);
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            scanner_advance(scanner);
            break;
        default:
            return;
        }
    }
}

static token_t scanner_token_create(scanner_t *scanner, token_type type)
{
    token_t token;
    token.type = type;
    token.start = scanner->token_start;
    token.length = scanner->input - scanner->token_start;
    return token;
}

static token_t scanner_token_error_create(const char *error_msg)
{
    token_t token;
    token.type = TOKEN_ERROR;
    token.start = error_msg;
    token.length = strlen(error_msg);
    return token;
}

static token_type scan_keyword(scanner_t *scanner, int start, int length, const char* suffix, token_type type)
{
    if (scanner->input - scanner->token_start != start + length)
        return TOKEN_IDENT;

    for (int i = start; i < start + length; i++) {
        if (tolower(scanner->token_start[i]) != suffix[i - start])
            return TOKEN_IDENT;
    }

    return type;
}

static token_type scan_ident_type(scanner_t *scanner)
{
    switch(scanner_peek_start(scanner)) {
    case 's': return scan_keyword(scanner, 1, 5, "elect", TOKEN_SELECT);
    case 'f': return scan_keyword(scanner, 1, 3, "rom", TOKEN_FROM);
    case 'w': return scan_keyword(scanner, 1, 4, "here", TOKEN_WHERE);
    case 'a': {
        /* either AND or ASC here */
        token_type t = scan_keyword(scanner, 1, 2, "nd", TOKEN_AND);
        if (t != TOKEN_IDENT)
            return t;

        return scan_keyword(scanner, 1, 2, "sc", TOKEN_ASC);;
    }
    case 'o': return scan_keyword(scanner, 1, 4, "rder", TOKEN_ORDER);
    case 'b': return scan_keyword(scanner, 1, 1, "y", TOKEN_BY);
    case 'd': return scan_keyword(scanner, 1, 3, "esc", TOKEN_DESC);
    }
    return TOKEN_IDENT;
}

static token_t scanner_ident(scanner_t *scanner)
{
    while (char_is_digit(scanner_peek(scanner)) || char_is_alpha(scanner_peek(scanner)))
        scanner_advance(scanner);

    return scanner_token_create(scanner, scan_ident_type(scanner));
}

static token_t scanner_number(scanner_t *scanner)
{
    while (char_is_digit(scanner_peek(scanner)))
        scanner_advance(scanner);

    return scanner_token_create(scanner, TOKEN_NUMBER);
}

token_t scanner_next(scanner_t *scanner)
{
    scanner_skip_space(scanner);

    scanner->token_start = scanner->input;

    if (scanner_at_eos(scanner))
        return scanner_token_create(scanner, TOKEN_EOS);

    char c = scanner_advance(scanner);

    if (char_is_alpha(c))
        return scanner_ident(scanner);

    if (char_is_digit(c))
        return scanner_number(scanner);

    switch (c) {
    case ';': return scanner_token_create(scanner, TOKEN_SEMICOLON);
    case ',': return scanner_token_create(scanner, TOKEN_COMMA);
    case '*': return scanner_token_create(scanner, TOKEN_STAR);
    case '=': return scanner_token_create(scanner, TOKEN_EQUAL);
    case '<': return scanner_token_create(scanner, TOKEN_LESS);
    case '>': return scanner_token_create(scanner, TOKEN_GREATER);
    }

    return scanner_token_error_create("Unknown character");
}


query_t *query_create(void)
{
    query_t *query = calloc(1, sizeof(*query));
    if (!query)
        return NULL;
    return query;
}

void query_destroy(query_t *query)
{
    if (query)
        free(query);
}

static void query_add_attr(query_t *query, token_t token)
{
    strncpy(query->attr_names[query->attr_num], token.start, (size_t)token.length);
    query->attr_num++;
}

static void query_add_rel(query_t *query, token_t token)
{
    strncpy(query->rel_names[query->rel_num], token.start, (size_t)token.length);
    query->rel_num++;
}

static void query_add_pred(query_t *query, token_t left_operand, token_t operator, token_t right_operand)
{
    query->predicates[query->pred_num].left = left_operand;
    query->predicates[query->pred_num].op = operator;
    query->predicates[query->pred_num].right = right_operand;
    query->pred_num++;
}

parser_t *parser_create(void)
{
    parser_t *parser = calloc(1, sizeof(*parser));
    if (!parser)
        return NULL;
    return parser;

}

void parser_destroy(parser_t *parser)
{
    if (parser)
        free(parser);
}

static void parser_error_at(parser_t *parser, token_t token, const char *msg)
{
    (void) token;
    if (parser->had_error)
        return;
    parser->had_error = true;

    fprintf(stderr, "Error: ");
    fprintf(stderr, "%s\n", msg);
}

static void parser_error(parser_t *parser, const char* msg) {
    parser_error_at(parser, parser->previous, msg);
}

static void parser_error_at_current(parser_t *parser, const char *msg)
{
    parser_error_at(parser, parser->current, msg);
}

static void parser_advance(parser_t *parser)
{
    parser->previous = parser->current;
    for (;;) {
        parser->current = scanner_next(parser->scanner);
        if (parser->current.type != TOKEN_ERROR)
            break;

        parser_error_at_current(parser, parser->current.start);
    }
}

static void parser_consume(parser_t *parser, token_type type, const char *msg)
{
    if (parser->current.type == type) {
        parser_advance(parser);
        return;
    }

    parser_error_at_current(parser, msg);
}

static bool parser_check(parser_t *parser, token_type type) {
    return parser->current.type == type;
}

static bool parser_match(parser_t *parser, token_type type)
{
    if (!parser_check(parser, type))
        return false;
    parser_advance(parser);
    return true;
}

static void parse_predicate(parser_t *parser)
{
    parser_consume(parser, TOKEN_IDENT, "Left predicate identifier expected");
    token_t left = parser->previous;

    if (!parser_match(parser, TOKEN_EQUAL) &&
        !parser_match(parser, TOKEN_LESS) &&
        !parser_match(parser, TOKEN_GREATER)) {
        parser_error(parser, "Predicate operator expected");
        return;
    }
    token_t op = parser->previous;

    if (!parser_match(parser, TOKEN_IDENT) &&
        !parser_match(parser, TOKEN_NUMBER)) {
        parser_error(parser, "Right predicate identifier or number expected");
        return;
    }
    token_t right = parser->previous;

    query_add_pred(parser->query, left, op, right);
}

static void parse_select(parser_t *parser)
{
    /* Collect attribute names */
    do {
        parser_consume(parser, TOKEN_IDENT, "Attribute name expected");
        query_add_attr(parser->query, parser->previous);
    } while (parser_match(parser, TOKEN_COMMA));

    /* Collect relation names */
    parser_consume(parser, TOKEN_FROM, "FROM expected");

    do {
        parser_consume(parser, TOKEN_IDENT, "Relation name expected");
        query_add_rel(parser->query, parser->previous);
    } while (parser_match(parser, TOKEN_COMMA));

    /* Collect filtering predicates */
    if (parser_match(parser, TOKEN_WHERE)) {
        do {
            parse_predicate(parser);
        } while (parser_match(parser, TOKEN_AND));
    }

    /* /\* Order by *\/ */
    /* if (parser_match(parser, TOKEN_ORDER_BY)) { */
    /*     parse_order(parser); */
    /* } */
    /* TODO: ORDER BY ident [ASC/DESC] */
}

static void parse_query(parser_t *parser)
{
    /* NOTE: we only match a single query type for now */
    if (parser_match(parser, TOKEN_SELECT))
        parse_select(parser);
    else
        parser_error(parser, "Only SELECT query is supported");

    parser_consume(parser, TOKEN_SEMICOLON, "Queries should end with a semicolon");
    parser_consume(parser, TOKEN_EOS, "Only single line queries are supported");
}

bool parser_parse(parser_t *parser, scanner_t *scanner, query_t *query)
{
     parser->scanner = scanner;
     parser->query = query;

     parser->had_error = false;

     parser_advance(parser);

     while (!parser_match(parser, TOKEN_EOS))
         parse_query(parser);

     return !parser->had_error;
}
