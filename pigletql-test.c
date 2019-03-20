#include <assert.h>
#include <stdlib.h>

#include "pigletql.h"

#define ARRAY_SIZE(arr) sizeof(arr) / sizeof((arr)[0])

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Check relation loaded from an in-memory table */
    {
        relation_t *relation = relation_create();
        assert(relation);

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);
        relation_fill_from_table(relation, &tuple_table[0][0], attr_names, tuple_num, attr_num);

        const uint16_t id_pos = relation_value_pos_by_name(relation, "id");
        const uint16_t attr1_pos = relation_value_pos_by_name(relation, "attr1");
        const uint16_t attr2_pos = relation_value_pos_by_name(relation, "attr2");

        /* Make sure all the values are there */
        value_type_t *tuple_start = relation_tuple_values_by_id(relation, 0);
        assert(tuple_start[id_pos] == 0);
        assert(tuple_start[attr1_pos] == 2);
        assert(tuple_start[attr2_pos] == 3);

        tuple_start = relation_tuple_values_by_id(relation, 1);
        assert(tuple_start[id_pos] == 1);
        assert(tuple_start[attr1_pos] == 12);
        assert(tuple_start[attr2_pos] == 13);

        tuple_start = relation_tuple_values_by_id(relation, 2);
        assert(tuple_start[id_pos] == 2);
        assert(tuple_start[attr1_pos] == 22);
        assert(tuple_start[attr2_pos] == 23);

        tuple_start = relation_tuple_values_by_id(relation, 3);
        assert(tuple_start[id_pos] == 3);
        assert(tuple_start[attr1_pos] == 32);
        assert(tuple_start[attr2_pos] == 33);

        relation_destroy(relation);
    }

    /* Check the basic relation scan operator */
    {
        relation_t *relation = relation_create();
        assert(relation);

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);
        relation_fill_from_table(relation, &tuple_table[0][0], attr_names, tuple_num, attr_num);

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

            scan_op_destroy(scan_op);
        }

        /* Check values received */
        {
            operator_t *scan_op = scan_op_create(relation);
            assert(scan_op);

            scan_op->open(scan_op->state);

            tuple_t *tuple = scan_op->next(scan_op->state);
            assert(tuple);
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "id") == 0);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);

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

            scan_op_destroy(scan_op);

        }

        relation_destroy(relation);
    }

    /* The projection operator */
    {
        relation_t *relation = relation_create();
        assert(relation);

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);
        relation_fill_from_table(relation, &tuple_table[0][0], attr_names, tuple_num, attr_num);

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

            assert(!tuple_has_attr(tuple, "id"));
            assert(tuple_has_attr(tuple, "attr1"));
            assert(tuple_has_attr(tuple, "attr2"));
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);

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

            proj_op_destroy(proj_op);
            scan_op_destroy(scan_op);
        }

        relation_destroy(relation);
    }

    /* union operator */
    {
        relation_t *left_relation = relation_create();
        relation_t *right_relation = relation_create();
        assert(left_relation);
        assert(right_relation);

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

        relation_fill_from_table(left_relation, &left_tuple_table[0][0], attr_names, left_tuple_num, attr_num);
        relation_fill_from_table(right_relation, &right_tuple_table[0][0], attr_names, right_tuple_num, attr_num);

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

            assert(tuple_get_attr_value(tuple, "id") == 0);
            assert(tuple_get_attr_value(tuple, "attr1") == 2);
            assert(tuple_get_attr_value(tuple, "attr2") == 3);

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

            scan_op_destroy(left_scan_op);
            scan_op_destroy(right_scan_op);
            union_op_destroy(union_op);
        }

        relation_destroy(left_relation);
        relation_destroy(right_relation);
    }

    /* Join operator */
    {
        relation_t *left_relation = relation_create();
        relation_t *right_relation = relation_create();
        assert(left_relation);
        assert(right_relation);

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

        relation_fill_from_table(
            left_relation,
            &left_tuple_table[0][0],
            left_attr_names,
            left_tuple_num,
            left_attr_num
        );
        relation_fill_from_table(
            right_relation,
            &right_tuple_table[0][0],
            right_attr_names,
            right_tuple_num,
            right_attr_num
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

            assert(tuple_get_attr_value(tuple, "attr1") == 1);
            assert(tuple_get_attr_value(tuple, "attr2") == 2);
            assert(tuple_get_attr_value(tuple, "attr3") == 31);
            assert(tuple_get_attr_value(tuple, "attr4") == 32);
            assert(tuple_get_attr_value(tuple, "attr5") == 33);

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

            scan_op_destroy(left_scan_op);
            scan_op_destroy(right_scan_op);
            join_op_destroy(join_op);
        }

        relation_destroy(left_relation);
        relation_destroy(right_relation);
    }

    /* Selection operator (attr to const comparison) */
    {
        relation_t *relation = relation_create();
        assert(relation);

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const value_type_t tuple_table[4][ARRAY_SIZE(attr_names)] = {
            {0, 02, 03},
            {1, 12, 13},
            {2, 22, 23},
            {3, 32, 33},
        };
        const uint32_t tuple_num = ARRAY_SIZE(tuple_table);
        const uint16_t attr_num = ARRAY_SIZE(attr_names);
        relation_fill_from_table(relation, &tuple_table[0][0], attr_names, tuple_num, attr_num);

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
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 1);

            tuple = select_op->next(select_op->state);
            assert(tuple);
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 2);

            tuple = select_op->next(select_op->state);
            assert(!tuple);

            select_op->close(select_op->state);

            select_op_destroy(select_op);
            scan_op_destroy(scan_op);
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

            select_op_destroy(select_op);
            scan_op_destroy(scan_op);
        }

        relation_destroy(relation);
    }

    /* Selection operator (attr to attr comparison) */
    {
        relation_t *relation = relation_create();
        assert(relation);

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
        relation_fill_from_table(relation, &tuple_table[0][0], attr_names, tuple_num, attr_num);

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
            assert(tuple_has_attr(tuple, "id"));
            assert(tuple_get_attr_value(tuple, "id") == 100);
            assert(tuple_get_attr_value(tuple, "attr1") == 50);
            assert(tuple_get_attr_value(tuple, "attr2") == 30);

            tuple = select_op->next(select_op->state);
            assert(!tuple);

            select_op->close(select_op->state);

            select_op_destroy(select_op);
            scan_op_destroy(scan_op);
        }

        relation_destroy(relation);
    }

    /* TODO: sort order */
    return 0;
}
