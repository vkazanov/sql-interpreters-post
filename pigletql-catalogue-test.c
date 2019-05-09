#include <assert.h>
#include <stdlib.h>

#include "pigletql-def.h"
#include "pigletql-catalogue.h"
#include "pigletql-eval.h"

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Check an empty catalogue and adding relations to it*/
    {
        catalogue_t *cat = catalogue_create();
        assert(cat);

        assert(NULL == catalogue_get_relation(cat, "rel1"));

        const attr_name_t attr_names[] = {"id", "attr1", "attr2"};
        const size_t attr_num = ARRAY_SIZE(attr_names);

        relation_t *rel1 = relation_create(attr_names, attr_num);
        catalogue_add_relation(cat, "rel1", rel1);

        rel1 = catalogue_get_relation(cat, "rel1");
        assert(rel1);
        assert(relation_get_attr_num(rel1) == attr_num);
        assert(relation_has_attr(rel1, "id"));
        assert(relation_has_attr(rel1, "attr1"));
        assert(relation_has_attr(rel1, "attr2"));

        relation_t *rel2 = relation_create(attr_names, attr_num);
        catalogue_add_relation(cat, "rel2", rel2);
        assert(catalogue_get_relation(cat, "rel2"));

        assert(!catalogue_get_relation(cat, "no rel"));

        catalogue_destroy(cat);
        relation_destroy(rel1);
        relation_destroy(rel2);
    }

    return 0;
}
