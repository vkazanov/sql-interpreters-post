#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "pigletql-parser.h"
#include "pigletql-eval.h"
#include "pigletql-catalogue.h"

/* We only have a single catalogue anyways so no need to pass it around */
static catalogue_t *cat;

void dump_predicate(const query_predicate_t *predicate)
{
    char buf[1024] = { 0 };
    strncat(buf, predicate->left.start, (size_t)predicate->left.length);
    strncat(buf, " ", 1);
    strncat(buf, predicate->op.start, (size_t)predicate->op.length);
    strncat(buf, " ", 1);
    strncat(buf, predicate->right.start, (size_t)predicate->right.length);
    printf("  %s,\n", buf);
}

void dump_select(const query_select_t *query)
{
    printf("SELECT\n");

    for (size_t i = 0; i < query->attr_num; ++i)
        printf("  %s,\n", query->attr_names[i]);

    printf("FROM\n");
    for (size_t i = 0; i < query->rel_num; ++i)
        printf("  %s,\n", query->rel_names[i]);

    if (!query->pred_num)
        goto order;

    printf("WHERE\n");
    for (size_t i = 0; i < query->pred_num; ++i)
        dump_predicate(&query->predicates[i]);

order:
    if (!query->has_order)
        return;

    printf("ORDER BY\n");
    printf("  %s\n", query->order_by_attr);
    printf(query->order_type == SORT_ASC ? "  ASC\n" : "  DESC\n");
}

void dump_create_table(const query_create_table_t *query)
{
    printf("CREATE TABLE \n");

    printf("  %s\n", query->rel_name);

    printf("(\n  ");
    for (size_t i = 0; i < query->attr_num; ++i)
        printf("%s,", query->attr_names[i]);
    printf("\n)\n");
}

void dump_insert(const query_insert_t *query)
{
    printf("INSERT INTO \n");

    printf("  %s\n", query->rel_name);

    printf("(\n  ");
    for (size_t i = 0; i < query->value_num; ++i)
        printf("%"PRI_VALUE",", query->values[i]);
    printf("\n)\n");
}


void dump(const query_t *query)
{
    switch (query->tag) {
    case QUERY_SELECT:
        dump_select(&query->as.select);
        break;
    case QUERY_CREATE_TABLE:
        dump_create_table(&query->as.create_table);
        break;
    case QUERY_INSERT:
        dump_insert(&query->as.insert);
        break;
    }
}

bool validate_select(const query_select_t *query)
{
    return true;
}

bool validate_create_table(const query_create_table_t *query)
{
    /* A relation should not exists */
    if (catalogue_get_relation(cat, query->rel_name)) {
        fprintf(stderr, "Error: relation '%s' already exists\n", query->rel_name);
        return false;
    }

    /* Attribute names are unique */
    for (size_t self_i = 0; self_i < query->attr_num; self_i++)
        for (size_t other_i = 0; other_i < query->attr_num; other_i++) {
            if (self_i == other_i)
                continue;
            if (0 != strncasecmp(query->attr_names[self_i], query->attr_names[other_i], MAX_ATTR_NAME_LEN))
                continue;

            const char *msg = "Error: duplicate relation attribute name '%s' at %zu and %zu\n";
            fprintf(stderr, msg, query->attr_names[self_i], self_i, other_i);
            return false;
        }

    return true;
}

bool validate_insert(const query_insert_t *query)
{
    /* A relation should exists */
    relation_t *target_rel = catalogue_get_relation(cat, query->rel_name);
    if (!target_rel) {
        fprintf(stderr, "Error: relation '%s' does not exist\n", query->rel_name);
        return false;
    }

    /* Number of attribute values to be inserted should be correct */
    uint16_t rel_attr_num = relation_get_attr_num(target_rel);
    uint16_t query_attr_num = query->value_num;
    if (rel_attr_num != query_attr_num) {
        fprintf(stderr, "Error: relation '%s' has %"PRIu16" attributes, only %"PRIu16" supplied\n",
                query->rel_name, rel_attr_num, query_attr_num);
        return false;
    }

    return true;
}

bool validate(const query_t *query)
{
    switch (query->tag) {
    case QUERY_SELECT:
        return validate_select(&query->as.select);
    case QUERY_CREATE_TABLE:
        return validate_create_table(&query->as.create_table);
    case QUERY_INSERT:
        return validate_insert(&query->as.insert);
     }
    assert(false);
}

void eval(const query_t *query)
{
    (void) query;
    /* TODO: compile the query into operators */
}

void run(const char *query_str)
{
    printf("%s\n", query_str);

    scanner_t *scanner = scanner_create(query_str);
    parser_t *parser = parser_create();
    query_t *query = query_create();

    if (parser_parse(parser, scanner, query)) {
        dump(query);
        if (validate(query))
            eval(query);
    }

    scanner_destroy(scanner);
    parser_destroy(parser);
    query_destroy(query);
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    cat = catalogue_create();

    while (true) {
        char line[1024];

        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* strip a newline at the end of the line */
        line[strlen(line) - 1] = '\0';

        run(line);
    }

    catalogue_destroy(cat);

    return 0;
}
