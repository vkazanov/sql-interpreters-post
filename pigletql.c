#include <stdlib.h>
#include <string.h>

#include "pigletql.h"

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
    const value_type_t **table,
    const attr_name_t *tuple_attr_names,
    const uint32_t table_tuple_num,
    const uint16_t tuple_attr_num)
{

}

value_type_t *relation_tuple_values_by_id(relation_t *rel, uint32_t tuple_i)
{
    return rel->tuples[tuple_i];
}

uint16_t relation_value_pos_by_name(relation_t *rel, const attr_name_t attr_name)
{
    for (size_t attr_i = 0; attr_i < rel->attr_num; attr_i++)
        if (strcmp(rel->attr_names[attr_i], attr_name) == 0)
            return attr_i;
    return ATTR_NOT_FOUND;
}

void relation_destroy(relation_t *rel)
{
    if (rel)
        free(rel);
}
