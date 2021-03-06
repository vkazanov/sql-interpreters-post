#ifndef PIGLETQL_EVAL_H
#define PIGLETQL_EVAL_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "pigletql-def.h"

/*
 * A tuple is a reference to a real tuple stored in a relation
 * */

typedef struct tuple_t tuple_t;

bool tuple_has_attr(const tuple_t *tuple, const attr_name_t attr_name);

value_type_t tuple_get_attr_value(const tuple_t *tuple, const attr_name_t attr_name);

uint16_t tuple_get_attr_num(const tuple_t *tuple);

value_type_t tuple_get_attr_value_by_i(const tuple_t *tuple, const uint16_t attr_i);

const char *tuple_get_attr_name_by_i(const tuple_t *tuple, const uint16_t attr_i);

/*
 * Relation is an in-memory table containing raw tuple data
 *  */

typedef struct relation_t relation_t;

relation_t *relation_create(const attr_name_t *attr_names, const uint16_t attr_num);

relation_t *relation_create_for_tuple(const tuple_t *tuple);

void relation_fill_from_table(relation_t *relation,
                              const value_type_t *table,
                              const uint32_t table_tuple_num);

void relation_order_by(relation_t *rel, const attr_name_t sort_attr_name, const sort_order_t order);

value_type_t *relation_tuple_values_by_id(const relation_t *rel, const uint32_t tuple_i);

uint16_t relation_attr_i_by_name(const relation_t *rel, const attr_name_t attr_name);

const char *relation_attr_name_by_i(const relation_t *rel, const uint16_t attr_i);

bool relation_has_attr(const relation_t *rel, const attr_name_t attr_name);

uint16_t relation_get_attr_num(const relation_t *rel);

uint32_t relation_get_tuple_num(const relation_t *rel);

void relation_append_tuple(relation_t *rel, const tuple_t *tuple);

void relation_append_values(relation_t *rel, const value_type_t *values);

void relation_reset(relation_t *relation);

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
 *
 * destroy - deallocates all the memory required by an operator and it's child operators
 * */

typedef struct operator_t operator_t;

typedef void (*op_open)(void *state);
typedef tuple_t *(*op_next)(void *state);
typedef void (*op_close)(void *state);
typedef void (*op_destroy)(operator_t *state);

/* The operator itself is just 4 pointers to related ops and operator state */
struct operator_t {
    op_open open;
    op_next next;
    op_close close;
    op_destroy destroy;

    void *state;
} ;

/*
 * Table scan operator just goes over all tuples in a relation.
 *  */

operator_t *scan_op_create(const relation_t *relation);

/*
 * Projection operator chooses a subset of attributes.
 *  */

operator_t *proj_op_create(operator_t *source,
                           const attr_name_t *tuple_attr_names,
                           const uint16_t tuple_attr_num);

/*
 * Union operator gets tuples from both supplied relations with the same attributes.
 * */

operator_t *union_op_create(operator_t *left_source,
                            operator_t *right_source);

/*
 * Join operator does a cross join of two relations, i.e. it returns all possible combinations of
 * tuples from both relations, with attributes joined.
 * */

operator_t *join_op_create(operator_t *left_source,
                           operator_t *right_source);


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

/*
 * Sort operator sorts tuples by a given attribute in ascending or descending order
 *  */

operator_t *sort_op_create(operator_t *source,
                           const attr_name_t sort_attr_name,
                           const sort_order_t order);

#endif //PIGLETQL_EVAL_H
