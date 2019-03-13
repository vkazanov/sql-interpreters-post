#ifndef PIGLETQL_H
#define PIGLETQL_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/*
 * Common definitions
 *  */

#define MAX_ATTR_NUM (UINT16_MAX - 1)
#define ATTR_NOT_FOUND UINT16_MAX
#define MAX_ATTR_NAME_LEN 256

typedef uint32_t value_type_t;  /* a single value type supported */
typedef char attr_name_t[MAX_ATTR_NAME_LEN]; /* attribute names are fixed-size strings */

/*
 * A tuple is a reference to a real tuple stored in a relation
 * */

typedef struct tuple_t tuple_t;

bool tuple_has_attr(const tuple_t *tuple, const attr_name_t attr_name);

value_type_t tuple_get_attr_value(const tuple_t *tuple, const attr_name_t attr_name);

/*
 * Relation is an in-memory table containing raw tuple data
 *  */

typedef struct relation_t relation_t;

relation_t *relation_create(void);

void relation_fill_from_table(relation_t *relation,
                              const value_type_t *table,
                              const attr_name_t *tuple_attr_names,
                              const uint32_t table_tuple_num,
                              const uint16_t tuple_attr_num);

value_type_t *relation_tuple_values_by_id(relation_t *rel, uint32_t tuple_i);

uint16_t relation_value_pos_by_name(relation_t *rel, const attr_name_t attr_name);

bool relation_has_attr(relation_t *rel, const attr_name_t attr_name);

void relation_destroy(relation_t *relation);

/*
 * Operators iterate over relation tuples or tuples returned from other operators using 3 standard
 * ops (open/next/close)
 * */

typedef void (*op_open)(void *state);
typedef tuple_t *(*op_next)(void *state);
typedef void (*op_close)(void *state);

typedef struct operator_t {
    op_open open;
    op_next next;
    op_close close;
    void *state;
} operator_t;

/*
 * Table scan operator just goes over all tuples in a relation
 *  */

operator_t *scan_op_create(relation_t *relation);

void scan_op_destroy(operator_t *operator);

#endif //PIGLETQL_H
