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

    return 0;
}
