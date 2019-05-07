#include <assert.h>
#include <string.h>

#include "pigletql-parser.h"

static void select_test(void)
{
    /* Basic SELECT scanner test */
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

    /* ident scanner error test */
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

    /* ORDER BY scanner test */
    {
        const char *query = "ORDER   BY ASC DESC";

        scanner_t *scanner = scanner_create(query);

        token_t token = scanner_next(scanner);
        assert(token.type == TOKEN_ORDER);
        assert(0 == strncmp(token.start, "ORDER", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_BY);
        assert(0 == strncmp(token.start, "BY", 2));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_ASC);
        assert(0 == strncmp(token.start, "ASC", 3));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_DESC);
        assert(0 == strncmp(token.start, "DESC", 4));

        scanner_destroy(scanner);
    }

    /* Basic SELECT queries */
    {
        const char *query_str = "SELECT attr1 FROM rel1;";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(scanner);
        assert(parser);
        assert(query);

        assert(parser_parse(parser, scanner, query));

        assert(query->tag == QUERY_SELECT);
        assert(query->as.select.attr_num == 1);
        assert(0 == strncmp(query->as.select.attr_names[0], "attr1", MAX_ATTR_NAME_LEN));
        assert(query->as.select.rel_num == 1);
        assert(0 == strncmp(query->as.select.rel_names[0], "rel1", MAX_REL_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    {
        const char *query_str = "SELECT attr1, attr2 FROM rel1,rel2,rel3;";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();

        assert(parser_parse(parser, scanner, query));

        assert(query->as.select.attr_num == 2);
        assert(0 == strncmp(query->as.select.attr_names[0], "attr1", MAX_ATTR_NAME_LEN));
        assert(0 == strncmp(query->as.select.attr_names[1], "attr2", MAX_ATTR_NAME_LEN));
        assert(query->as.select.rel_num == 3);
        assert(0 == strncmp(query->as.select.rel_names[0], "rel1", MAX_REL_NAME_LEN));
        assert(0 == strncmp(query->as.select.rel_names[1], "rel2", MAX_REL_NAME_LEN));
        assert(0 == strncmp(query->as.select.rel_names[2], "rel3", MAX_REL_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    {
        const char *query_str = "SELECT a1 FROM r1 WHERE a1=a2 AND b2<3 AND b3>4;";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();

        assert(parser_parse(parser, scanner, query));

        assert(query->as.select.pred_num == 3);

        assert(query->as.select.predicates[0].left.type == TOKEN_IDENT);
        assert(query->as.select.predicates[0].op.type == TOKEN_EQUAL);
        assert(query->as.select.predicates[0].right.type == TOKEN_IDENT);

        assert(query->as.select.predicates[1].left.type == TOKEN_IDENT);
        assert(query->as.select.predicates[1].op.type == TOKEN_LESS);
        assert(query->as.select.predicates[1].right.type == TOKEN_NUMBER);

        assert(query->as.select.predicates[2].left.type == TOKEN_IDENT);
        assert(query->as.select.predicates[2].op.type == TOKEN_GREATER);
        assert(query->as.select.predicates[2].right.type == TOKEN_NUMBER);

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    {
        const char *query_str = "SELECT a1, a2 FROM r1 ORDER BY a3 DESC;";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();

        assert(parser_parse(parser, scanner, query));
        assert(query->tag == QUERY_SELECT);
        assert(query->as.select.has_order);
        assert(query->as.select.order_type == SORT_DESC);
        assert(0 == strncmp(query->as.select.order_by_attr, "a3", MAX_ATTR_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        query_str = "SELECT a1, a2 FROM r1 ORDER BY a3;";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(parser_parse(parser, scanner, query));
        assert(query->as.select.has_order);
        assert(query->as.select.order_type == SORT_ASC);
        assert(0 == strncmp(query->as.select.order_by_attr, "a3", MAX_ATTR_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        query_str = "SELECT a1, a2 FROM r1 ORDER BY a3 ASC;";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(parser_parse(parser, scanner, query));
        assert(query->as.select.has_order);
        assert(query->as.select.order_type == SORT_ASC);
        assert(0 == strncmp(query->as.select.order_by_attr, "a3", MAX_ATTR_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }
}

static void create_test(void)
{

}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    select_test();
    create_test();

    return 0;
}
