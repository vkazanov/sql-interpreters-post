#include <assert.h>
#include <stdlib.h>

#include "pigletql-eval.h"

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Check relation loaded from an in-memory table */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        const uint16_t id_i = relation_attr_i_by_name(relation, "id");
        const uint16_t attr1_i = relation_attr_i_by_name(relation, "attr1");
        const uint16_t attr2_i = relation_attr_i_by_name(relation, "attr2");

        /* Make sure all the values are there */
        value_type_t *tuple_start = relation_tuple_values_by_id(relation, 0);
        assert(tuple_start[id_i] == 0);
        assert(tuple_start[attr1_i] == 2);
        assert(tuple_start[attr2_i] == 3);

        tuple_start = relation_tuple_values_by_id(relation, 1);
        assert(tuple_start[id_i] == 1);
        assert(tuple_start[attr1_i] == 12);
        assert(tuple_start[attr2_i] == 13);

        tuple_start = relation_tuple_values_by_id(relation, 2);
        assert(tuple_start[id_i] == 2);
        assert(tuple_start[attr1_i] == 22);
        assert(tuple_start[attr2_i] == 23);

        tuple_start = relation_tuple_values_by_id(relation, 3);
        assert(tuple_start[id_i] == 3);
        assert(tuple_start[attr1_i] == 32);
        assert(tuple_start[attr2_i] == 33);

        relation_destroy(relation);
    }

    /* Check appending raw values to a relation */
    {
        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        const value_type_t values[] = {1, 2, 3};

        relation_append_values(relation, values);
        assert(relation_get_tuple_num(relation) == 1);

        value_type_t *tuple_start = relation_tuple_values_by_id(relation, 0);
        assert(tuple_start[0] == 1);
        assert(tuple_start[1] == 2);
        assert(tuple_start[2] == 3);

        relation_append_values(relation, values);
        assert(relation_get_tuple_num(relation) == 2);
        tuple_start = relation_tuple_values_by_id(relation, 1);
        assert(tuple_start[0] == 1);
        assert(tuple_start[1] == 2);
        assert(tuple_start[2] == 3);

        relation_destroy(relation);
    }

    /* Check the basic relation scan operator */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        /* Count the tuples twice */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            scan_op->open(scan_op->state);
            size_t tuples_received = 0;
            tuple_t *tuple = NULL;
            while((tuple = scan_op->next(scan_op->state)))
                tuples_received++;
            scan_op->close(scan_op->state);
            assert(tuples_received == 4);

            scan_op->open(scan_op->state);
            tuples_received = 0;
            tuple = NULL;
            while((tuple = scan_op->next(scan_op->state)))
                tuples_received++;
            scan_op->close(scan_op->state);
            assert(tuples_received == 4);

            scan_op->destroy(scan_op);
        }

        /* Check values received */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            scan_op->open(scan_op->state);

            tuple_t *tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);

            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "id") == 0);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 0);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value_by_i(tuple, 1) == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);
            assert(tuple_get_attr_value_by_i(tuple, 2) == 3);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 1);
            assert(tuple_get_attr_value(tuple, "attr1") == 12);
            assert(tuple_get_attr_value(tuple, "attr2") == 13);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 2);
            assert(tuple_get_attr_value(tuple, "attr1") == 22);
            assert(tuple_get_attr_value(tuple, "attr2") == 23);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 3);
            assert(tuple_get_attr_value(tuple, "attr1") == 32);
            assert(tuple_get_attr_value(tuple, "attr2") == 33);

            tuple = scan_op->next(scan_op->state);
            assert(!tuple);

            scan_op->close(scan_op->state);

            scan_op->destroy(scan_op);

        }

        relation_destroy(relation);
    }

    /* Scan relation tuples into another relation */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation_source = relation_create(attr_names, attr_num);
        assert(relation_source);

        relation_fill_from_table(relation_source, &tuple_table[0][0], tuple_num);

        relation_t *relation_target = relation_create(attr_names, attr_num);
        /* Scan the source and fill the target */
        {
            operator_t *scan_op_source = scan_op_create(relation_source);
            assert(scan_op_source);

            scan_op_source->open(scan_op_source->state);

            tuple_t *tuple = NULL;
            while((tuple = scan_op_source->next(scan_op_source->state)))
                relation_append_tuple(relation_target, tuple);

            scan_op_source->close(scan_op_source->state);

            scan_op_source->destroy(scan_op_source);
        }

        /* Check target tuples */
        {
            operator_t *scan_op = scan_op_create(relation_target);
            assert(scan_op);

            scan_op->open(scan_op->state);

            tuple_t *tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);

            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "id") == 0);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 0);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value_by_i(tuple, 1) == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);
            assert(tuple_get_attr_value_by_i(tuple, 2) == 3);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 1);
            assert(tuple_get_attr_value(tuple, "attr1") == 12);
            assert(tuple_get_attr_value(tuple, "attr2") == 13);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 2);
            assert(tuple_get_attr_value(tuple, "attr1") == 22);
            assert(tuple_get_attr_value(tuple, "attr2") == 23);

            tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 3);
            assert(tuple_get_attr_value(tuple, "attr1") == 32);
            assert(tuple_get_attr_value(tuple, "attr2") == 33);

            tuple = scan_op->next(scan_op->state);
            assert(!tuple);

            scan_op->close(scan_op->state);

            scan_op->destroy(scan_op);
        }

        relation_destroy(relation_target);
        relation_destroy(relation_source);
    }

    /* The projection operator */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        /* Count the tuples, check attributes */
        {
            /* This is gonna be wrapped */
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            const attr_name_t projected_attr_list[] = {"attr1", "attr2"};

            operator_t *proj_op = proj_op_create(scan_op,
                                                 projected_attr_list,
                                                 ARRAY_SIZE(projected_attr_list));
            assert(proj_op);

            proj_op->open(proj_op->state);

            tuple_t *tuple = proj_op->next(proj_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 2);

            assert(!tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);
            assert(tuple_get_attr_value_by_i(tuple, 1) == 3);

            tuple = proj_op->next(proj_op->state);
            assert(tuple);

            assert(!tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "attr1") == 12);
            assert(tuple_get_attr_value(tuple, "attr2") == 13);

            tuple = proj_op->next(proj_op->state);
            assert(tuple);

            assert(!tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "attr1") == 22);
            assert(tuple_get_attr_value(tuple, "attr2") == 23);

            tuple = proj_op->next(proj_op->state);
            assert(tuple);

            assert(!tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "attr1") == 32);
            assert(tuple_get_attr_value(tuple, "attr2") == 33);

            tuple = proj_op->next(proj_op->state);
            assert(!tuple);

            proj_op->close(proj_op->state);

            proj_op->destroy(proj_op);
        }

        relation_destroy(relation);
    }

    /* union operator */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const uint16_t attr_num = ARRAY_SIZE(attr_names);
        const value_type_t left_tuple_table[3][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
        };
        const value_type_t right_tuple_table[2][ARRAY_SIZE(attr_names)] = {
            {3, 32, 33},
            {4, 42, 43},
        };
        const uint32_t left_tuple_num = ARRAY_SIZE(left_tuple_table);
        const uint32_t right_tuple_num = ARRAY_SIZE(right_tuple_table);

        relation_t *left_relation = relation_create(attr_names, attr_num);
        relation_t *right_relation = relation_create(attr_names, attr_num);
        assert(left_relation);
        assert(right_relation);

        relation_fill_from_table(left_relation, &left_tuple_table[0][0], left_tuple_num);
        relation_fill_from_table(right_relation, &right_tuple_table[0][0], right_tuple_num);

        {
            operator_t *left_scan_op = scan_op_create(left_relation);
            operator_t *right_scan_op = scan_op_create(right_relation);
            assert(left_scan_op);
            assert(right_scan_op);

            operator_t *union_op = union_op_create(left_scan_op, right_scan_op);
            assert(union_op);

            union_op->open(union_op->state);

            tuple_t *tuple = union_op->next(union_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);

            assert(tuple_get_attr_value(tuple, "id") == 0);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 0);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value_by_i(tuple, 1) == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);
            assert(tuple_get_attr_value_by_i(tuple, 2) == 3);

            tuple = union_op->next(union_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "id") == 1);
            assert(tuple_get_attr_value(tuple, "attr1") == 12);
            assert(tuple_get_attr_value(tuple, "attr2") == 13);

            tuple = union_op->next(union_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "id") == 2);
            assert(tuple_get_attr_value(tuple, "attr1") == 22);
            assert(tuple_get_attr_value(tuple, "attr2") == 23);

            tuple = union_op->next(union_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "id") == 3);
            assert(tuple_get_attr_value(tuple, "attr1") == 32);
            assert(tuple_get_attr_value(tuple, "attr2") == 33);

            tuple = union_op->next(union_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "id") == 4);
            assert(tuple_get_attr_value(tuple, "attr1") == 42);
            assert(tuple_get_attr_value(tuple, "attr2") == 43);

            tuple = union_op->next(union_op->state);
            assert(!tuple);

            union_op->close(union_op->state);

            union_op->destroy(union_op);
        }

        relation_destroy(left_relation);
        relation_destroy(right_relation);
    }

    /* Join operator */
    {

        const attr_name_t left_attr_names[] = {"attr1", "attr2"};
        const uint16_t left_attr_num = ARRAY_SIZE(left_attr_names);
        const value_type_t left_tuple_table[3][ARRAY_SIZE(left_attr_names)] = {
            {01, 02},
            {11, 12},
            {21, 22},
        };
        const uint32_t left_tuple_num = ARRAY_SIZE(left_tuple_table);

        const attr_name_t right_attr_names[] = {"attr3", "attr4", "attr5"};
        const uint16_t right_attr_num = ARRAY_SIZE(right_attr_names);
        const value_type_t right_tuple_table[2][ARRAY_SIZE(right_attr_names)] = {
            {31, 32, 33},
            {41, 42, 43},
        };
        const uint32_t right_tuple_num = ARRAY_SIZE(right_tuple_table);

        relation_t *left_relation = relation_create(left_attr_names, left_attr_num);
        relation_t *right_relation = relation_create(right_attr_names, right_attr_num);
        assert(left_relation);
        assert(right_relation);

        relation_fill_from_table(
            left_relation,
            &left_tuple_table[0][0],
            left_tuple_num
        );
        relation_fill_from_table(
            right_relation,
            &right_tuple_table[0][0],
            right_tuple_num
        );

        {
            operator_t *left_scan_op = scan_op_create(left_relation);
            operator_t *right_scan_op = scan_op_create(right_relation);
            assert(left_scan_op);
            assert(right_scan_op);

            operator_t *join_op = join_op_create(left_scan_op, right_scan_op);
            assert(join_op);

            join_op->open(join_op->state);

            tuple_t *tuple = join_op->next(join_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 5);

            assert(tuple_get_attr_value(tuple, "attr1") == 1);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 1);
            assert(tuple_get_attr_value(tuple, "attr2") == 2);
            assert(tuple_get_attr_value_by_i(tuple, 1) == 2);
            assert(tuple_get_attr_value(tuple, "attr3") == 31);
            assert(tuple_get_attr_value_by_i(tuple, 2) == 31);
            assert(tuple_get_attr_value(tuple, "attr4") == 32);
            assert(tuple_get_attr_value_by_i(tuple, 3) == 32);
            assert(tuple_get_attr_value(tuple, "attr5") == 33);
            assert(tuple_get_attr_value_by_i(tuple, 4) == 33);

            tuple = join_op->next(join_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "attr1") == 1);
            assert(tuple_get_attr_value(tuple, "attr2") == 2);
            assert(tuple_get_attr_value(tuple, "attr3") == 41);
            assert(tuple_get_attr_value(tuple, "attr4") == 42);
            assert(tuple_get_attr_value(tuple, "attr5") == 43);

            tuple = join_op->next(join_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "attr1") == 11);
            assert(tuple_get_attr_value(tuple, "attr2") == 12);
            assert(tuple_get_attr_value(tuple, "attr3") == 31);
            assert(tuple_get_attr_value(tuple, "attr4") == 32);
            assert(tuple_get_attr_value(tuple, "attr5") == 33);

            tuple = join_op->next(join_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "attr1") == 11);
            assert(tuple_get_attr_value(tuple, "attr2") == 12);
            assert(tuple_get_attr_value(tuple, "attr3") == 41);
            assert(tuple_get_attr_value(tuple, "attr4") == 42);
            assert(tuple_get_attr_value(tuple, "attr5") == 43);

            tuple = join_op->next(join_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "attr1") == 21);
            assert(tuple_get_attr_value(tuple, "attr2") == 22);
            assert(tuple_get_attr_value(tuple, "attr3") == 31);
            assert(tuple_get_attr_value(tuple, "attr4") == 32);
            assert(tuple_get_attr_value(tuple, "attr5") == 33);

            tuple = join_op->next(join_op->state);
            assert(tuple);

            assert(tuple_get_attr_value(tuple, "attr1") == 21);
            assert(tuple_get_attr_value(tuple, "attr2") == 22);
            assert(tuple_get_attr_value(tuple, "attr3") == 41);
            assert(tuple_get_attr_value(tuple, "attr4") == 42);
            assert(tuple_get_attr_value(tuple, "attr5") == 43);

            tuple = join_op->next(join_op->state);
            assert(!tuple);

            join_op->close(join_op->state);

            join_op->destroy(join_op);
        }

        relation_destroy(left_relation);
        relation_destroy(right_relation);
    }

    /* Selection operator (attr to const comparison) */
    {

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            operator_t *select_op = select_op_create(scan_op);
            assert(select_op);

            /* Select rows where: 0 < id < 3  */
            select_op_add_attr_const_predicate(select_op, "id", SELECT_GT, 0);
            select_op_add_attr_const_predicate(select_op, "id", SELECT_LT, 3);

            select_op->open(select_op->state);

            tuple_t *tuple = select_op->next(select_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 1);
            assert(tuple_get_attr_value_by_i(tuple, 0) == 1);

            tuple = select_op->next(select_op->state);
            assert(tuple);
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 2);

            tuple = select_op->next(select_op->state);
            assert(!tuple);

            select_op->close(select_op->state);

            select_op->destroy(select_op);
        }

        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            operator_t *select_op = select_op_create(scan_op);
            assert(select_op);

            /* Select row where: attr1 = 12  */
            select_op_add_attr_const_predicate(select_op, "attr1", SELECT_EQ, 12);

            select_op->open(select_op->state);

            tuple_t *tuple = select_op->next(select_op->state);
            assert(tuple);
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_get_attr_value(tuple, "attr1") == 12);

            tuple = select_op->next(select_op->state);
            assert(!tuple);

            select_op->close(select_op->state);

            select_op->destroy(select_op);
        }

        relation_destroy(relation);
    }

    /* Selection operator (attr to attr comparison) */
    {
        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[5][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {100, 50, 30},
            {150, 30, 50},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        /* A single check */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            operator_t *select_op = select_op_create(scan_op);
            assert(select_op);

            /* Select row where: id > attr1 AND attr1 > attr2   */
            select_op_add_attr_attr_predicate(select_op, "id", SELECT_GT, "attr1");
            select_op_add_attr_attr_predicate(select_op, "attr1", SELECT_GT, "attr2");

            select_op->open(select_op->state);

            tuple_t *tuple = select_op->next(select_op->state);

            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 100);
            assert(tuple_get_attr_value(tuple, "attr1") == 50);
            assert(tuple_get_attr_value(tuple, "attr2") == 30);

            tuple = select_op->next(select_op->state);
            assert(!tuple);

            select_op->close(select_op->state);

            select_op->destroy(select_op);
        }

        relation_destroy(relation);
    }

    /* Sort operator */
    {
        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {9, 02, 03},
            {5, 12, 13},
            {7, 22, 23},
            {2, 50, 30},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *relation = relation_create(attr_names, attr_num);
        assert(relation);

        relation_fill_from_table(relation, &tuple_table[0][0], tuple_num);

        /* Ascending order */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            operator_t *sort_op = sort_op_create(scan_op, "id", SORT_ASC);
            assert(sort_op);

            sort_op->open(sort_op->state);

            tuple_t *tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);
            assert(tuple_get_attr_value(tuple, "id") == 2);
            assert(tuple_get_attr_value(tuple, "attr1") == 50);
            assert(tuple_get_attr_value(tuple, "attr2") == 30);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 5);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 7);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 9);

            tuple = sort_op->next(sort_op->state);
            assert(!tuple);

            sort_op->close(sort_op->state);

            sort_op->destroy(sort_op);
        }

        /* Descending order */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            operator_t *sort_op = sort_op_create(scan_op, "id", SORT_DESC);
            assert(sort_op);

            sort_op->open(sort_op->state);

            tuple_t *tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_num(tuple) == 3);
            assert(tuple_get_attr_value(tuple, "id") == 9);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 7);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 5);

            tuple = sort_op->next(sort_op->state);
            assert(tuple);
            assert(tuple_get_attr_value(tuple, "id") == 2);

            tuple = sort_op->next(sort_op->state);
            assert(!tuple);

            sort_op->close(sort_op->state);

            sort_op->destroy(sort_op);
        }

        relation_destroy(relation);
    }
    return 0;
}
