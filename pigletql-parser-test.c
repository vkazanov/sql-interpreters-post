#include <assert.h>
#include <string.h>

#include "pigletql-parser.h"

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Basic scanner test */
    {
        const char *query = "SELECT *,attr1 FROM WHERE attr1=11;";

        scanner_t *scanner = scanner_create(query);

        token_t token = scanner_next(scanner);
        assert(token.type == TOKEN_SELECT);
        assert(0 == strncmp(token.start, "SELECT", 6));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_STAR);
        assert(0 == strncmp(token.start, "*", 1));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_COMMA);
        assert(0 == strncmp(token.start, ",", 1));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_IDENT);
        assert(0 == strncmp(token.start, "attr1", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_FROM);
        assert(0 == strncmp(token.start, "FROM", 4));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_WHERE);
        assert(0 == strncmp(token.start, "WHERE", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_IDENT);
        assert(0 == strncmp(token.start, "attr1", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_EQUAL);
        assert(0 == strncmp(token.start, "=", 1));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_NUMBER);
        assert(0 == strncmp(token.start, "11", 2));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_SEMICOLON);
        assert(0 == strncmp(token.start, ";", 1));

        scanner_destroy(scanner);
    }

    /* Basic scanner error */
    {
        const char *query = "attr1 !1attr";

        scanner_t *scanner = scanner_create(query);

        token_t token = scanner_next(scanner);
        assert(token.type == TOKEN_IDENT);
        assert(0 == strncmp(token.start, "attr1", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_ERROR);
        const char *err_msg = "Unknown character";
        assert(0 == strncmp(token.start, err_msg, strlen(err_msg)));

        scanner_destroy(scanner);
    }

    /* TODO: */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2"; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2"; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2 WHERE a1=a2 "; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2 WHERE a1=a2 ORDER BY a3 DESC/ASC"; */

    return 0;
}
