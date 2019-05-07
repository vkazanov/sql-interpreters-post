#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

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

static void create_table_test(void)
{
    /* table scanner test */
    {
        const char *query = "CREATE TABLE (a1);";

        scanner_t *scanner = scanner_create(query);

        token_t token = scanner_next(scanner);
        assert(token.type == TOKEN_CREATE);
        assert(0 == strncmp(token.start, "CREATE", 6));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_TABLE);
        assert(0 == strncmp(token.start, "TABLE", 5));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_LPAREN);
        assert(0 == strncmp(token.start, "(", 1));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_IDENT);
        assert(0 == strncmp(token.start, "a1", 2));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_RPAREN);
        assert(0 == strncmp(token.start, ")", 1));

        scanner_destroy(scanner);
    }

    /* basic CREATE TABLE query test */
    {
        const char *query_str = "CREATE TABLE rel1 (a1, a2);";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(scanner);
        assert(parser);
        assert(query);

        assert(parser_parse(parser, scanner, query));

        assert(query->tag == QUERY_CREATE_TABLE);

        assert(query->as.create_table.attr_num == 2);
        assert(0 == strncmp(query->as.create_table.attr_names[0], "a1", MAX_ATTR_NAME_LEN));
        assert(0 == strncmp(query->as.create_table.attr_names[1], "a2", MAX_ATTR_NAME_LEN));

        assert(0 == strncmp(query->as.create_table.rel_name, "rel1", MAX_REL_NAME_LEN));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

}

static void insert_test(void)
{
    /* insert scanner test */
    {
        const char *query = "INSERT INTO rel1 VALUES (111);";

        scanner_t *scanner = scanner_create(query);

        token_t token = scanner_next(scanner);
        assert(token.type == TOKEN_INSERT);
        assert(0 == strncmp(token.start, "INSERT", 6));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_INTO);
        assert(0 == strncmp(token.start, "INTO", 4));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_IDENT);
        assert(0 == strncmp(token.start, "rel1", 4));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_VALUES);
        assert(0 == strncmp(token.start, "VALUES", 6));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_LPAREN);

        token = scanner_next(scanner);
        assert(token.type == TOKEN_NUMBER);
        assert(0 == strncmp(token.start, "111", 3));

        token = scanner_next(scanner);
        assert(token.type == TOKEN_RPAREN);

        scanner_destroy(scanner);
    }

    /* basic INSERT INTO query test */
    {
        const char *query_str = "INSERT INTO rel1 VALUES (111, 222);";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(scanner);
        assert(parser);
        assert(query);

        assert(parser_parse(parser, scanner, query));

        assert(query->tag == QUERY_INSERT);

        assert(0 == strncmp(query->as.insert.rel_name, "rel1", MAX_REL_NAME_LEN));

        assert(query->as.insert.value_num == 2);
        assert(query->as.insert.values[0] == 111);
        assert(query->as.insert.values[1] == 222);

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

}

static void error_test(void)
{
    /* Block stderr output to avoid err msg spamming */
    int null_fd = open("/dev/null", O_WRONLY);
    int stderr_fd = dup(2);
    dup2(null_fd, 2);

    /* INSERT errors */
    {
        /* No INTO */
        const char *query_str = "INSERT rel1 VALUES (111, 222);";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(scanner);
        assert(parser);
        assert(query);

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        /* No VALUES */
        query_str = "INSERT INTO rel1 (111, 222);";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        /* Invalid value list */
        query_str = "INSERT INTO rel1 VALUES ();";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        query_str = "INSERT INTO rel1 VALUES (111,);";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    /* CREATE TABLE errors */
    {
        const char *query_str = "CREATE rel1;";

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        query_str = "CREATE TABLE rel1;";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);

        query_str = "CREATE TABLE rel1 (a1,);";

        scanner = scanner_create(query_str);
        parser = parser_create();
        query = query_create();

        assert(!parser_parse(parser, scanner, query));

        scanner_destroy(scanner);
        parser_destroy(parser);
        query_destroy(query);
    }

    /* Get back normal stderr */
    dup2(stderr_fd, 2);
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    select_test();
    create_table_test();
    insert_test();

    error_test();

    return 0;
}
