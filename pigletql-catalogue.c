#include <stdlib.h>
#include <string.h>

#include "pigletql-catalogue.h"

typedef struct record_t {
    rel_name_t name;
    relation_t *relation;
    struct record_t *next;
} record_t;

typedef struct catalogue_t {
    record_t *record_list;
} catalogue_t;

catalogue_t *catalogue_create(void)
{
    catalogue_t *cat = calloc(1, sizeof(*cat));
    if (!cat)
        return NULL;

    return cat;
}

void catalogue_destroy(catalogue_t *cat)
{
    for (record_t **this = &cat->record_list; *this; this = &(*this)->next)
        free(*this);
    free(cat);
}

relation_t *catalogue_get_relation(catalogue_t *cat, const rel_name_t rel_name)
{
    for (record_t **this = &cat->record_list; *this; this = &(*this)->next)
        if (0 == strncmp((*this)->name, rel_name, MAX_REL_NAME_LEN))
            return (*this)->relation;
    return NULL;
}

relation_t *catalogue_add_relation(catalogue_t *cat, const rel_name_t rel_name, relation_t *rel)
{
    record_t *record = calloc(1, sizeof(*record));
    if (!record)
        return NULL;
    strncpy(record->name, rel_name, MAX_REL_NAME_LEN);
    record->relation = rel;

    record_t **this = &cat->record_list;
    for (; *this; this = &(*this)->next);
    *this = record;

    return rel;
}
