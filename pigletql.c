#include <stdlib.h>

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

void relation_destroy(relation_t *rel)
{
    if (rel)
        free(rel);
}
