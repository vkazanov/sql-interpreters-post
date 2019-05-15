#include <unistd.h>
#include <fcntl.h>

#include "pigletql-validate.h"

static void create_validate_test(void)
{
    /* Correctly create a table */
    {
        const char *query_str = "CREATE TABLE rel1 (a1, a2);";

        catalogue_t *cat = catalogue_create();
        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }

    /* Create a table with non-unique attributes */
    {
        const char *query_str = "CREATE TABLE rel1 (a1, a1);";

        catalogue_t *cat = catalogue_create();
        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(!validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }

    /* A table that already exists */
    {
        const char *query_str = "CREATE TABLE rel1 (a1, a1);";

        catalogue_t *cat = catalogue_create();
        {
            const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
            const size_t attr_num = ARRAY_SIZE(attr_names);
            relation_t *rel1 = relation_create(attr_names, attr_num);
            catalogue_add_relation(cat, "rel1", rel1);
        }

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(!validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }
}

static void insert_validate_test(void)
{
    /* Insert into a non-existing table */
    {
        const char *query_str = "INSERT INTO rel1 VALUES (1, 2);";

        catalogue_t *cat = catalogue_create();
        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(!validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }

    /* Correct insertion */
    {
        const char *query_str = "INSERT INTO rel1 VALUES (1, 2, 3);";

        catalogue_t *cat = catalogue_create();
        {
            const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
            const size_t attr_num = ARRAY_SIZE(attr_names);
            relation_t *rel1 = relation_create(attr_names, attr_num);
            catalogue_add_relation(cat, "rel1", rel1);
        }

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }

    /* Insert into a table with a different number of columns */
    {
        const char *query_str = "INSERT INTO rel1 VALUES (2, 3);";

        catalogue_t *cat = catalogue_create();
        {
            const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
            const size_t attr_num = ARRAY_SIZE(attr_names);
            relation_t *rel1 = relation_create(attr_names, attr_num);
            catalogue_add_relation(cat, "rel1", rel1);
        }

        scanner_t *scanner = scanner_create(query_str);
        parser_t *parser = parser_create();
        query_t *query = query_create();
        assert(cat);
        assert(scanner);
        assert(parser);
        assert(query);
        assert(parser_parse(parser, scanner, query));

        assert(!validate(cat, query));

        query_destroy(query);
        parser_destroy(parser);
        scanner_destroy(scanner);
        catalogue_destroy(cat);
    }

}

static void select_validate_test(void)
{

}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Block stderr output to avoid err msg spamming */
    int null_fd = open("/dev/null", O_WRONLY);
    int stderr_fd = dup(2);
    dup2(null_fd, 2);

    create_validate_test();
    insert_validate_test();
    select_validate_test();

    /* Get back normal stderr */
    dup2(stderr_fd, 2);

    return 0;
}
