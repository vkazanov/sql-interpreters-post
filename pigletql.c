#include <stdlib.h>
#include <string.h>

#include "pigletql.h"

/*
 * Tuple represents either a tuple itself, a tuple projection or a tuple join
 *  */

typedef enum tuple_tag {
    TUPLE_SOURCE,
    TUPLE_PROJECT,
    TUPLE_JOIN
} tuple_tag;

/* Source tuple is a reference to raw data in the relations */
typedef struct tuple_source_t {
    /* A reference to a relation containing the tuple */
    const relation_t *relation;
    /* A reference to the values in the relation containing the tuple */
    value_type_t *values;
} tuple_source_t;

/* A projected tuple is a reference to another tuple giving access to a subset of referenced tuple
 * attributes only */
typedef struct tuple_project_t {
    /* a reference to tuple to project attributes from  */
    tuple_t *source_tuple;
    /* projected attributes */
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;
} tuple_project_t;

/* A joined tuple a tuple referencing  */
typedef struct tuple_join_t {
    /* Contained tuples to join attributes from */
    tuple_t *left_source_tuple;
    tuple_t *right_source_tuple;
} tuple_join_t;

/* A unified tuple type passed between operators */
struct tuple_t {
    tuple_tag tag;
    union {
        tuple_source_t source;
        tuple_project_t project;
        tuple_join_t join;
    } as;
};

static bool tuple_source_has_attr(const tuple_source_t *source, const attr_name_t attr_name)
{
    return relation_has_attr(source->relation, attr_name);
}

static bool tuple_project_has_attr(const tuple_project_t *project, const attr_name_t attr_name)
{
    for (size_t attr_i = 0; attr_i < project->attr_num; attr_i++ )
        if (strcmp(project->attr_names[attr_i], attr_name) == 0)
            return tuple_has_attr(project->source_tuple, attr_name);
    return false;
}

static bool tuple_join_has_attr(const tuple_join_t *join, const attr_name_t attr_name)
{
    return tuple_has_attr(join->left_source_tuple, attr_name) ||
        tuple_has_attr(join->right_source_tuple, attr_name);
}

bool tuple_has_attr(const tuple_t *tuple, const attr_name_t attr_name)
{
    if (tuple->tag == TUPLE_SOURCE)
        return tuple_source_has_attr(&tuple->as.source, attr_name);
    else if (tuple->tag == TUPLE_PROJECT)
        return tuple_project_has_attr(&tuple->as.project, attr_name);
    else if (tuple->tag == TUPLE_JOIN)
        return tuple_join_has_attr(&tuple->as.join, attr_name);
    else
        assert(false);
}

static value_type_t tuple_source_get_attr_value(const tuple_source_t *source, const attr_name_t attr_name)
{
    const relation_t *relation = source->relation;
    uint16_t value_pos = relation_value_pos_by_name(relation, attr_name);
    return source->values[value_pos];
}

static value_type_t tuple_project_get_attr_value(const tuple_project_t *project, const attr_name_t attr_name)
{
    for (size_t attr_i = 0; attr_i < project->attr_num; attr_i++ )
        if (strcmp(project->attr_names[attr_i], attr_name) == 0)
            return tuple_get_attr_value(project->source_tuple, attr_name);
    assert(false);
}

static value_type_t tuple_join_get_attr_value(const tuple_join_t *join, const attr_name_t attr_name)
{
    if (tuple_has_attr(join->left_source_tuple, attr_name))
        return tuple_get_attr_value(join->left_source_tuple, attr_name);
    else if (tuple_has_attr(join->right_source_tuple, attr_name))
        return tuple_get_attr_value(join->right_source_tuple, attr_name);
    else
        assert(false);
}

value_type_t tuple_get_attr_value(const tuple_t *tuple, const attr_name_t attr_name)
{
    if (tuple->tag == TUPLE_SOURCE)
        return tuple_source_get_attr_value(&tuple->as.source, attr_name);
    else if (tuple->tag == TUPLE_PROJECT)
        return tuple_project_get_attr_value(&tuple->as.project, attr_name);
    else if (tuple->tag == TUPLE_JOIN)
        return tuple_join_get_attr_value(&tuple->as.join, attr_name);
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

value_type_t *relation_tuple_values_by_id(const relation_t *rel, uint32_t tuple_i)
{
    return &rel->tuples[tuple_i * rel->attr_num];
}

uint16_t relation_value_pos_by_name(const relation_t *rel, const attr_name_t attr_name)
{
    for (size_t attr_i = 0; attr_i < rel->attr_num; attr_i++)
        if (strcmp(rel->attr_names[attr_i], attr_name) == 0)
            return attr_i;
    return ATTR_NOT_FOUND;
}

bool relation_has_attr(const relation_t *rel, const attr_name_t attr_name)
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

/* Table scanning operator */

typedef struct scan_op_state_t {
    /* A reference to the relation being scanned */
    const relation_t *relation;
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

operator_t *scan_op_create(const relation_t *relation)
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

/* Projection operator */

typedef struct proj_op_state_t {
    /* A reference to the operator to retrieve tuples from */
    operator_t *source;
    /* A projecting tuple  */
    tuple_t current_tuple;
} proj_op_state_t;

void proj_op_open(void *state)
{
    proj_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *source = op_state->source;
    source->open(source->state);
    op_state->current_tuple.as.project.source_tuple = NULL;
}

tuple_t *proj_op_next(void *state)
{
    proj_op_state_t *op_state = (typeof(op_state)) state;

    operator_t *source = op_state->source;
    tuple_t *next_source_tuple = source->next(source->state);
    if (!next_source_tuple)
        return NULL;
    op_state->current_tuple.as.project.source_tuple = next_source_tuple;

    return &op_state->current_tuple;
}

void proj_op_close(void *state)
{
    proj_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *source = op_state->source;
    op_state->current_tuple.as.project.source_tuple = NULL;
    source->close(source->state);
}

operator_t *proj_op_create(operator_t *source,
                           const attr_name_t *attr_names,
                           const uint16_t attr_num)
{
    assert(source);
    assert(attr_num > 0);

    operator_t *op = calloc(1, sizeof(*op));
    if (!op)
        goto op_fail;

    proj_op_state_t *state = calloc(1, sizeof(*state));
    if (!state)
        goto state_fail;

    state->current_tuple.tag = TUPLE_PROJECT;
    state->source = source;
    op->state = state;

    state->current_tuple.as.project.attr_num = attr_num;
    for (size_t i = 0; i < attr_num; ++i)
        strncpy(state->current_tuple.as.project.attr_names[i], attr_names[i], MAX_ATTR_NAME_LEN);

    op->open = proj_op_open;
    op->next = proj_op_next;
    op->close = proj_op_close;

    return op;

state_fail:
    free(op);
op_fail:
    return NULL;
}

void proj_op_destroy(operator_t *operator)
{
    if (!operator)
        return;
    free(operator->state);
    free(operator);
}

/* Union operator */

typedef struct union_op_state_t {
    /* Tuple sources to be united */
    operator_t *left_source;
    operator_t *right_source;

    /* the current tuple source */
    operator_t *current_source;
} union_op_state_t;

void union_op_open(void *state)
{
    union_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *left_source = op_state->left_source;
    operator_t *right_source = op_state->right_source;
    left_source->open(left_source->state);
    right_source->open(right_source->state);

    /* Begin with the left source */
    op_state->current_source = left_source;
}

tuple_t *union_op_next(void *state)
{
    union_op_state_t *op_state = (typeof(op_state)) state;

    operator_t *current_source = op_state->current_source;
    tuple_t *next_source_tuple = current_source->next(current_source->state);

    /* the left tuple source is exhausted: switch to the right source, reretrieve a tuple */
    if (!next_source_tuple && current_source == op_state->left_source) {
        op_state->current_source = op_state->right_source;
        current_source = op_state->current_source;

        next_source_tuple = current_source->next(current_source->state);
    }

    return next_source_tuple;
}

void union_op_close(void *state)
{
    union_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *left_source = op_state->left_source;
    operator_t *right_source = op_state->right_source;
    left_source->close(left_source->state);
    right_source->close(right_source->state);

    op_state->current_source = NULL;
}

operator_t *union_op_create(operator_t *left_source,
                            operator_t *right_source)
{
    assert(left_source && right_source);

    operator_t *op = calloc(1, sizeof(*op));
    if (!op)
        goto op_fail;

    union_op_state_t *state = calloc(1, sizeof(*state));
    if (!state)
        goto state_fail;

    state->left_source = left_source;
    state->right_source = right_source;
    op->state = state;

    op->open = union_op_open;
    op->next = union_op_next;
    op->close = union_op_close;

    return op;

state_fail:
    free(op);
op_fail:
    return NULL;
}

void union_op_destroy(operator_t *operator)
{
    if (!operator)
        return;
    free(operator->state);
    free(operator);
}

/* Join operator */

typedef struct join_op_state_t {
    /* Tuple sources to be joined */
    operator_t *left_source;
    operator_t *right_source;

    /* Current left source tuple to be joined with right source tuples */
    tuple_t *current_left_tuple;

    /* Joined tuple to be returned */
    tuple_t current_tuple;
} join_op_state_t;

void join_op_open(void *state)
{
    join_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *left_source = op_state->left_source;
    operator_t *right_source = op_state->right_source;
    left_source->open(left_source->state);
    right_source->open(right_source->state);

    op_state->current_tuple.as.join.left_source_tuple = left_source->next(left_source->state);
}

tuple_t *join_op_next(void *state)
{
    join_op_state_t *op_state = (typeof(op_state)) state;

    operator_t *left_source = op_state->left_source;
    operator_t *right_source = op_state->right_source;
    tuple_join_t *join_tuple = &op_state->current_tuple.as.join;

    /* No more tuples in the left source? Done joining. */
    if (!join_tuple->left_source_tuple)
        return NULL;

    join_tuple->right_source_tuple = right_source->next(right_source->state);
    /* No more tuples in the right source? Try the next left source */
    if (!join_tuple->right_source_tuple) {
        join_tuple->left_source_tuple = left_source->next(left_source->state);
        /* TODO: reset the right source and get the first tuple there */
    }

    return &op_state->current_tuple;
}

void join_op_close(void *state)
{
    join_op_state_t *op_state = (typeof(op_state)) state;
    operator_t *left_source = op_state->left_source;
    operator_t *right_source = op_state->right_source;
    left_source->close(left_source->state);
    right_source->close(right_source->state);
}

operator_t *join_op_create(operator_t *left_source,
                           operator_t *right_source)
{
    assert(left_source && right_source);
    operator_t *op = calloc(1, sizeof(*op));
    if (!op)
        goto op_fail;

    join_op_state_t *state = calloc(1, sizeof(*state));
    if (!state)
        goto state_fail;

    state->left_source = left_source;
    state->right_source = right_source;
    state->current_tuple.tag = TUPLE_JOIN;
    op->state = state;

    op->open = join_op_open;
    op->next = join_op_next;
    op->close = join_op_close;

    return op;

state_fail:
    free(op);
op_fail:
    return NULL;
}

void join_op_destroy(operator_t *operator)
{
    if (!operator)
        return;
    free(operator->state);
    free(operator);
}
