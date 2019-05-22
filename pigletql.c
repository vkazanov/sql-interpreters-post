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

operator_t *compile_select(catalogue_t *cat, const query_select_t *query)
{
    operator_t *root_op = NULL;

    /* 1. Scan ops */
    /* 2. Join ops*/

    {
        size_t rel_i = 0;
        relation_t *rel = catalogue_get_relation(cat, query->rel_names[rel_i]);
        root_op = scan_op_create(rel);
        rel_i += 1;

        for (; rel_i < query->rel_num; rel_i++) {
            rel = catalogue_get_relation(cat, query->rel_names[rel_i]);
            operator_t *scan_op = scan_op_create(rel);
            root_op = join_op_create(root_op, scan_op);
        }
    }

    /* 3. Project */
    root_op = proj_op_create(root_op, query->attr_names, query->attr_num);

    /* 4. Select */
    if (query->pred_num > 0) {
        operator_t *select_op = select_op_create(root_op);
        for (size_t pred_i = 0; pred_i < query->pred_num; pred_i++) {
            query_predicate_t predicate = query->predicates[pred_i];

            /* On the left we always get an identifier */
            assert(predicate.left.type == TOKEN_IDENT);

            attr_name_t left_attr_name = {0};
            strncpy(left_attr_name, predicate.left.start, (size_t)predicate.left.length);

            select_predicate_op pred_op = 0;
            switch (predicate.op.type) {
            case TOKEN_GREATER:
                pred_op = SELECT_GT;
                break;
            case TOKEN_LESS:
                pred_op = SELECT_LT;
                break;
            case TOKEN_EQUAL:
                pred_op = SELECT_EQ;
                break;
            default:
                /* Uknown predicate type */
                assert(false);
            }

            /* On the right it's either a constant or another identifier */
            if (predicate.right.type == TOKEN_IDENT) {
                attr_name_t right_attr_name = {0};
                strncpy(right_attr_name, predicate.right.start, (size_t)predicate.right.length);

                select_op_add_attr_attr_predicate(select_op, left_attr_name, pred_op, right_attr_name);
            } else if (predicate.right.type == TOKEN_NUMBER) {
                char buf[128] = {0};
                strncpy(buf, predicate.right.start, (size_t)predicate.right.length);

                value_type_t right_const = 0;
                sscanf(buf, "%" SCN_VALUE, &right_const);

                select_op_add_attr_const_predicate(select_op, left_attr_name, pred_op, right_const);
            } else {
                /* Invalid token */
                assert(false);
            }
        }
        root_op = select_op;
    }

    /* 5. Sort */
    if (query->has_order)
        root_op = sort_op_create(root_op, query->order_by_attr, query->order_type);

    return root_op;
}

void dump_tuple_header(tuple_t *tuple)
{
    const uint16_t attr_num = tuple_get_attr_num(tuple);

    for (uint16_t attr_i = 0; attr_i < attr_num; attr_i++) {
        const char *attr_name = tuple_get_attr_name_by_i(tuple, attr_i);
        if (attr_i != attr_num - 1)
            printf("%s ", attr_name);
        else
            printf("%s\n", attr_name);
    }
}

void dump_tuple(tuple_t *tuple)
{
    const uint16_t attr_num = tuple_get_attr_num(tuple);

    /* attribute values for all rows */
    for (uint16_t attr_i = 0; attr_i < attr_num; attr_i++) {
        value_type_t attr_val = tuple_get_attr_value_by_i(tuple, attr_i);
        if (attr_i != attr_num - 1)
            printf("%u ", attr_val);
        else
            printf("%u\n", attr_val);
    }
}

bool eval_select(catalogue_t *cat, const query_select_t *query)
{
    /* Compile the operator tree:  */
    operator_t *root_op = compile_select(cat, query);


    /* Eval the tree: */
    {
        root_op->open(root_op->state);

        size_t tuples_received = 0;
        tuple_t *tuple = NULL;
        while((tuple = root_op->next(root_op->state))) {
            /* attribute list for the first row only */
            if (tuples_received == 0)
                dump_tuple_header(tuple);

            /* A table of tuples */
            dump_tuple(tuple);

            tuples_received++;
        }
        printf("rows: %zu\n", tuples_received);

        root_op->close(root_op->state);
    }

    root_op->destroy(root_op);

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
