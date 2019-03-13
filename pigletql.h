#ifndef PIGLETQL_H
#define PIGLETQL_H

#include <stdint.h>
#include <assert.h>

#define MAX_ATTR_NUM (UINT16_MAX - 1)
#define ATTR_NOT_FOUND UINT16_MAX
#define MAX_ATTR_NAME_LEN 256

typedef uint32_t value_type_t;
typedef char attr_name_t[MAX_ATTR_NAME_LEN];

typedef struct operator_t operator_t;
typedef struct operator_state_t operator_state_t;

typedef struct tuple_t tuple_t;
typedef struct tuple_proxy_t tuple_proxy_t;
typedef struct tuple_source_t tuple_source_t;
typedef enum tuple_tag tuple_tag;

struct operator_state_t {
    operator_t *left_input;
    operator_t *right_input;
    void *state;
};

typedef void (*op_open)(operator_state_t *state);
typedef tuple_t *(*op_next)(operator_state_t *state);
typedef void (*op_close)(operator_state_t *state);

struct operator_t {
    op_open open;
    op_next next;
    op_close close;
    operator_state_t *state;
};

enum tuple_tag {
    TUPLE_SOURCE,
    TUPLE_PROXY
};

struct tuple_proxy_t {
};

struct tuple_source_t {
};

struct tuple_t {
    tuple_tag tag;
    union {
        tuple_source_t source;
        tuple_proxy_t proxy;
    } as;
};

/*
 * Relation is an in-memory prefilled with values
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

void relation_destroy(relation_t *relation);

/*
 * Table scan operator just goes over all tuples in a relation
 *  */

#endif //PIGLETQL_H
