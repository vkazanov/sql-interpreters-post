#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "pigletql-parser.h"
#include "pigletql-eval.h"
#include "pigletql-catalogue.h"
#include "pigletql-validate.h"

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


bool eval_select(catalogue_t *cat, const query_select_t *query)
{
    (void) query; (void) cat;

    return true;
}

bool eval_create_table(catalogue_t *cat, const query_create_table_t *query)
{
    relation_t *rel = relation_create(query->attr_names, query->attr_num);
    if (!rel)
        goto rel_err;

    if (!catalogue_add_relation(cat, query->rel_name, rel))
        goto cat_err;

    return true;

cat_err:
    relation_destroy(rel);

rel_err:
    return false;
}

bool eval_insert(catalogue_t *cat, const query_insert_t *query)
{
    relation_t *rel = catalogue_get_relation(cat, query->rel_name);
    assert(rel);                /* should be validated by now */

    relation_append_values(rel, query->values);

    return true;
}

bool eval(catalogue_t *cat, const query_t *query)
{
     switch (query->tag) {
     case QUERY_SELECT:
         return eval_select(cat, &query->as.select);
     case QUERY_CREATE_TABLE:
         return eval_create_table(cat, &query->as.create_table);
     case QUERY_INSERT:
         return eval_insert(cat, &query->as.insert);
     }
     assert(false);
 }

void run(catalogue_t *cat, const char *query_str)
{
    scanner_t *scanner = scanner_create(query_str);
    parser_t *parser = parser_create();
    query_t *query = query_create();

    if (parser_parse(parser, scanner, query)) {
        dump(query);
        if (validate(cat, query))
            eval(cat, query);
    }

    scanner_destroy(scanner);
    parser_destroy(parser);
    query_destroy(query);
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    catalogue_t *cat = catalogue_create();

    while (true) {
        char line[1024];

        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* strip a newline at the end of the line */
        line[strlen(line) - 1] = '\0';

        run(cat, line);
    }

    catalogue_destroy(cat);

    return 0;
}
