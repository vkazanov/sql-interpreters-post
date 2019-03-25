#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "pigletql-parser.h"
#include "pigletql-def.h"

typedef struct query_t {
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;

    rel_name_t rel_names[MAX_REL_NUM];
    uint16_t rel_num;
} query_t;

typedef struct scanner_t {
    const char *input;
    const char *token_start;
} scanner_t;

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

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || c == '_';
}

static bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static bool is_eos(scanner_t *scanner)
{
    return *scanner->input == '\0';
}

static char peek(scanner_t *scanner)
{
    return tolower(*scanner->input);
}

static char peek_start(scanner_t *scanner)
{
    return tolower(*scanner->token_start);
}

static char advance(scanner_t *scanner)
{
    return *scanner->input++;
}

static void skip_space(scanner_t *scanner)
{
    for (;;) {
        char c = peek(scanner);
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance(scanner);
            break;
        default:
            return;
        }
    }
}

static token_t token_create(scanner_t *scanner, token_type type)
{
    token_t token;
    token.type = type;
    token.start = scanner->token_start;
    token.length = scanner->input - scanner->token_start;
    return token;
}

static token_type keyword(scanner_t *scanner, int start, int length, const char* suffix, token_type type)
{
    if (scanner->input - scanner->token_start != start + length)
        return TOKEN_IDENT;

    for (int i = start; i < start + length; i++) {
        if (tolower(scanner->token_start[i]) != suffix[i - start])
            return TOKEN_IDENT;
    }

    return type;
}

static token_type ident_type(scanner_t *scanner)
{
    switch(peek_start(scanner)) {
    case 's': return keyword(scanner, 1, 5, "elect", TOKEN_SELECT);
    case 'f': return keyword(scanner, 1, 3, "rom", TOKEN_FROM);
    case 'w': return keyword(scanner, 1, 4, "here", TOKEN_WHERE);
    }
    return TOKEN_IDENT;
}

static token_t ident(scanner_t *scanner)
{
    while (is_digit(peek(scanner)) || is_alpha(peek(scanner)))
        advance(scanner);

    return token_create(scanner, ident_type(scanner));
}

static token_t number(scanner_t *scanner)
{
    while (is_digit(peek(scanner)))
        advance(scanner);

    return token_create(scanner, TOKEN_NUMBER);
}

token_t scanner_next(scanner_t *scanner)
{
    skip_space(scanner);

    scanner->token_start = scanner->input;

    if (is_eos(scanner)) return token_create(scanner, TOKEN_EOS);

    char c = peek(scanner);
    advance(scanner);

    if (is_alpha(c))
        return ident(scanner);

    if (is_digit(c))
        return number(scanner);

    switch (c) {
    case ';': return token_create(scanner, TOKEN_SEMICOLON);
    case ',': return token_create(scanner, TOKEN_COMMA);
    case '*': return token_create(scanner, TOKEN_STAR);
    case '=': return token_create(scanner, TOKEN_EQUAL);
    }

    assert(false);
}

void parse(const char *query_string, query_t *query)
{
    (void) query_string; (void) query;

    scanner_t *scanner = scanner_create(query_string);

    scanner_destroy(scanner);
}
