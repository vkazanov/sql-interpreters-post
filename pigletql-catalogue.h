#ifndef PIGLETQL_CATALOGUE_H
#define PIGLETQL_CATALOGUE_H

#include <stdbool.h>

#include "pigletql-def.h"
#include "pigletql-eval.h"

typedef struct catalogue_t catalogue_t;

catalogue_t *catalogue_create(void);

void catalogue_destroy(catalogue_t *catalogue);

relation_t *catalogue_get_relation(catalogue_t *catalogue, const rel_name_t rel_name);

relation_t *catalogue_add_relation(catalogue_t *catalogue, const rel_name_t rel_name, relation_t *rel);

#endif //PIGLETQL_CATALOGUE_H
