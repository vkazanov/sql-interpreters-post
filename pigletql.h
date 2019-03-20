#ifndef PIGLETQL_H
#define PIGLETQL_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/*
 * Common definitions
 *  */

/* maximum number of attributes per relation */
#define MAX_ATTR_NUM (UINT16_MAX - 1)
/* return value telling that an attribute was not found */
#define ATTR_NOT_FOUND UINT16_MAX
/* maximum attribute name length */
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

value_type_t *relation_tuple_values_by_id(const relation_t *rel, uint32_t tuple_i);

uint16_t relation_value_pos_by_name(const relation_t *rel, const attr_name_t attr_name);

bool relation_has_attr(const relation_t *rel, const attr_name_t attr_name);

void relation_destroy(relation_t *relation);

/*
 * Operators iterate over relation tuples or tuples returned from other operators using 3 standard
 * ops: open, next, close.
 *
 * open - starts iteration over available tuples from the very beginning
 *
 * next - retrieves the next tuple, ending with a NULL value
 *
 * close - closes the operator and resets its state
 * */

typedef void (*op_open)(void *state);
typedef tuple_t *(*op_next)(void *state);
typedef void (*op_close)(void *state);

/* The operator itself is just 3 pointers to related ops and operator state */
typedef struct operator_t {
    op_open open;
    op_next next;
    op_close close;
    void *state;
} operator_t;

/*
 * Table scan operator just goes over all tuples in a relation.
 *  */

operator_t *scan_op_create(const relation_t *relation);

void scan_op_destroy(operator_t *operator);

/*
 * Projection operator chooses a subset of attributes.
 *  */

operator_t *proj_op_create(operator_t *source,
                           const attr_name_t *tuple_attr_names,
                           const uint16_t tuple_attr_num);

void proj_op_destroy(operator_t *operator);

/*
 * Union operator gets tuples from both supplied relations with the same attributes.
 * */

operator_t *union_op_create(operator_t *left_source,
                            operator_t *right_source);

void union_op_destroy(operator_t *operator);

/*
 * Join operator does a cross join of two relations, i.e. it returns all possible combinations of
 * tuples from both relations, with attributes joined.
 * */

operator_t *join_op_create(operator_t *left_source,
                           operator_t *right_source);

void join_op_destroy(operator_t *operator);


/*
 * Selection operator filters tuples according to a list of predicates
 *  */

typedef enum select_predicate_op {
    SELECT_GT,                  /* greater than */
    SELECT_LT,                  /* less than  */
    SELECT_EQ                   /* equal */
} select_predicate_op;

void select_op_add_attr_const_predicate(operator_t *operator,
                                        const attr_name_t left_attr_name,
                                        const select_predicate_op predicate_op,
                                        const value_type_t right_constant);

void select_op_add_attr_attr_predicate(operator_t *operator,
                                       const attr_name_t left_attr_name,
                                       const select_predicate_op predicate_op,
                                       const attr_name_t right_attr_name);

operator_t *select_op_create(operator_t *source);

void select_op_destroy(operator_t *operator);

/*
 * Sort operator sorts tuples by a given attribute in ascending or descending order
 *  */

typedef enum sort_op_order {
    SORT_ASC,
    SORT_DESC,
} sort_op_order;

operator_t *sort_op_create(operator_t *source, attr_name_t sort_attr_name, sort_op_order sort_order);

void sort_op_destroy(operator_t *operator);

#endif //PIGLETQL_H
