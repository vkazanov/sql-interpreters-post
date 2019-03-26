#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "pigletql-parser.h"

typedef struct scanner_t {
    const char *input;
    const char *token_start;
} scanner_t;

typedef struct parser_t {
    const scanner_t *scanner;
    const query_t *query;
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
/* static void parser_consume(parser_t *parser, token_type type, const char *msg) */
/* { */

/* } */

void parser_parse(parser_t *parser, scanner_t *scanner, query_t *query)
{
    parser->scanner = scanner;
    parser->query = query;
}
