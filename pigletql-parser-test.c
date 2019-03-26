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

    /* Basic SELECT queries */
    {
        const char *query_str = "SELECT attr1 FROM table1;";

        scanner_t *scanner = scanner_create(query_str);
        assert(scanner);
        query_t *query = query_create();
        assert(query);
        parser_t *parser = parser_create();
        assert(parser);

        parser_parse(parser, scanner, query);

        assert(query->attr_num == 1);
        assert(0 == strncmp(query->attr_names[0], "attr1", MAX_ATTR_NAME_LEN));
        assert(query->rel_num == 1);
        assert(0 == strncmp(query->rel_names[0], "table1", MAX_REL_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    /* { */
    /*     const char *query_str = "SELECT attr1, attr2 FROM table;"; */
    /* } */

    /* { */
    /*     const char *query_str = "SELECT * FROM table1;"; */
    /* } */

    /* { */
    /*     const char *query_str = "SELECT * FROM table1, table2;"; */
    /* } */

    /* TODO: */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2"; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2"; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2 WHERE a1=a2 "; */
    /* const char *q1 = "SELECT a1, a2, a3 FROM r1, r2 WHERE a1=a2 ORDER BY a3 DESC/ASC"; */

    return 0;
}
