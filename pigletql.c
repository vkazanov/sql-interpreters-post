#include <stdlib.h>
#include <string.h>

#include "pigletql.h"

/*
 * Tuple represent either a tuple itself or a proxy limiting access to certain attributes
 *  */

typedef struct tuple_proxy_t {
} tuple_proxy_t;

typedef struct tuple_source_t {
    /* A reference to a relation containing the tuple */
    relation_t *relation;
    /* A reference to the values in the relation containing the tuple */
    value_type_t *values;
} tuple_source_t;


typedef enum tuple_tag {
    TUPLE_SOURCE,
    TUPLE_PROXY
} tuple_tag;

struct tuple_t {
    tuple_tag tag;
    union {
        /* Source tuple is a reference to raw data in the relations */
        tuple_source_t source;
        /* A proxy tuple is a reference to another tuple giving access to a subset of child
         * attributes only */
        tuple_proxy_t proxy;
    } as;
};

static bool tuple_source_has_attr(const tuple_source_t *source, const attr_name_t attr_name)
{
    return relation_has_attr(source->relation, attr_name);
}

bool tuple_has_attr(const tuple_t *tuple, const attr_name_t attr_name)
{
    /* TODO: We only support source tuples for now */
    if (tuple->tag == TUPLE_SOURCE)
        return tuple_source_has_attr(&tuple->as.source, attr_name);
    else if (tuple->tag == TUPLE_PROXY)
        assert(false);
    else
        assert(false);
}

static value_type_t tuple_source_get_attr_value(const tuple_source_t *source, const attr_name_t attr_name)
{
    relation_t *relation = source->relation;
    uint16_t value_pos = relation_value_pos_by_name(relation, attr_name);
    return source->values[value_pos];
}

value_type_t tuple_get_attr_value(const tuple_t *tuple, const attr_name_t attr_name)
{
    /* TODO: We only support source tuples for now */
    if (tuple->tag == TUPLE_SOURCE)
        return tuple_source_get_attr_value(&tuple->as.source, attr_name);
    else if (tuple->tag == TUPLE_PROXY)
        assert(false);
    else
        assert(false);
}

/*
 * Relation - see pigletql.h for comments
 *  */

struct relation_t {
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;

    value_type_t *tuples;
    uint32_t tuple_num;
};

relation_t *relation_create(void)
{
    relation_t *rel = calloc(1, sizeof(*rel));
    if (!rel)
        return NULL;
    *rel = (typeof(*rel)) { 0 };
    return rel;
}

void relation_fill_from_table(
    relation_t *rel,
    const value_type_t *table,
    const attr_name_t *attr_names,
    const uint32_t tuple_num,
    const uint16_t attr_num
)
{
    rel->attr_num = attr_num;
    for(size_t attr_i = 0; attr_i < attr_num; attr_i++)
            strncpy(rel->attr_names[attr_i], attr_names[attr_i], MAX_ATTR_NAME_LEN);

    rel->tuple_num = tuple_num;
    rel->tuples = calloc(1, sizeof(value_type_t) * attr_num * tuple_num);
    assert(rel->tuples);
    for(size_t tuple_i = 0; tuple_i < tuple_num; tuple_i++)
        for(size_t attr_i = 0; attr_i < attr_num; attr_i++)
            rel->tuples[tuple_i * attr_num + attr_i] = table[tuple_i * attr_num + attr_i];
}

value_type_t *relation_tuple_values_by_id(relation_t *rel, uint32_t tuple_i)
{
    return &rel->tuples[tuple_i * rel->attr_num];
}

uint16_t relation_value_pos_by_name(relation_t *rel, const attr_name_t attr_name)
{
    for (size_t attr_i = 0; attr_i < rel->attr_num; attr_i++)
        if (strcmp(rel->attr_names[attr_i], attr_name) == 0)
            return attr_i;
    return ATTR_NOT_FOUND;
}

bool relation_has_attr(relation_t *rel, const attr_name_t attr_name)
{
    return relation_value_pos_by_name(rel, attr_name) != ATTR_NOT_FOUND;
}

void relation_destroy(relation_t *rel)
{
    if (!rel)
        return;
    if (rel->tuples)
        free(rel->tuples);
    free(rel);
}

/*
 * Operators - see pigletql.h
 *  */

typedef struct scan_op_state_t {
    /* A reference to the relation being scanned */
    relation_t *relation;
    /* Next tuple index to retrieve from the relation */
    uint32_t next_tuple_i;
    /* A structure to be filled with references to tuple data */
    tuple_t current_tuple;
} scan_op_state_t;

void scan_op_open(void *state)
{
    scan_op_state_t *op_state = (typeof(op_state)) state;
    op_state->next_tuple_i = 0;
    tuple_t *current_tuple = &op_state->current_tuple;
    current_tuple->as.source.values = NULL;
}

tuple_t *scan_op_next(void *state)
{
    scan_op_state_t *op_state = (typeof(op_state)) state;
    if (op_state->next_tuple_i >= op_state->relation->tuple_num)
        return NULL;
    uint32_t current_i = op_state->next_tuple_i;
    tuple_source_t *source_tuple = &op_state->current_tuple.as.source;
    source_tuple->values = relation_tuple_values_by_id(op_state->relation, current_i);

    op_state->next_tuple_i++;

    return &op_state->current_tuple;
}

void scan_op_close(void *state)
{
    scan_op_state_t *op_state = (typeof(op_state)) state;
    op_state->next_tuple_i = 0;
    tuple_t *current_tuple = &op_state->current_tuple;
    current_tuple->as.source.values = NULL;
}

operator_t *scan_op_create(relation_t *relation)
{
    operator_t *op = calloc(1, sizeof(*op));
    if (!op)
        goto op_fail;

    scan_op_state_t *state = calloc(1, sizeof(*state));
    if (!state)
        goto state_fail;

    state->relation = relation;
    state->next_tuple_i = 0;
    state->current_tuple.tag = TUPLE_SOURCE;
    state->current_tuple.as.source.values = NULL;
    state->current_tuple.as.source.relation = relation;
    op->state = state;

    op->open = scan_op_open;
    op->next = scan_op_next;
    op->close = scan_op_close;

    return op;

state_fail:
        free(op);
op_fail:
    return NULL;
}

void scan_op_destroy(operator_t *operator)
{
    if (!operator)
        return;
    free(operator->state);
    free(operator);
}
