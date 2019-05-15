#include "pigletql-validate.h"

static bool attr_in_attr_names(const attr_name_t attr_name, const attr_name_t *attr_names, const uint16_t attr_num)
{
    for (size_t attr_i = 0; attr_i < attr_num; attr_i++) {
        if (0 != strncmp(attr_names[attr_i], attr_name, MAX_ATTR_NAME_LEN))
            continue;
        return true;
    }
    return false;

}

static bool attr_names_unique(const attr_name_t *attr_names, const uint16_t attr_num)
{
    for (size_t self_i = 0; self_i < attr_num; self_i++)
        for (size_t other_i = 0; other_i < attr_num; other_i++) {
            if (self_i == other_i)
                continue;
            if (0 != strncasecmp(attr_names[self_i], attr_names[other_i], MAX_ATTR_NAME_LEN))
                continue;

            const char *msg = "Error: duplicate attribute name '%s' at %zu and %zu\n";
            fprintf(stderr, msg, attr_names[self_i], self_i, other_i);

            return false;
        }
    return true;
}

static bool rel_names_unique(const rel_name_t *rel_names, const uint16_t rel_num)
{
    for (size_t self_i = 0; self_i < rel_num; self_i++)
        for (size_t other_i = 0; other_i < rel_num; other_i++) {
            if (self_i == other_i)
                continue;
            if (0 != strncasecmp(rel_names[self_i], rel_names[other_i], MAX_REL_NAME_LEN))
                continue;

            const char *msg = "Error: duplicate relation name '%s' at %zu and %zu\n";
            fprintf(stderr, msg, rel_names[self_i], self_i, other_i);

            return false;
        }
    return true;
}

static bool validate_select(catalogue_t *cat, const query_select_t *query)
{
    /* All the relations should exist */
    for (size_t rel_i = 0; rel_i < query->rel_num; rel_i++) {
        if (catalogue_get_relation(cat, query->rel_names[rel_i]))
            continue;

        fprintf(stderr, "Error: relation '%s' does not exist\n", query->rel_names[rel_i]);
        return false;
    }

    /* Relation names should be unique */
    if (!rel_names_unique(query->rel_names,query->rel_num))
        return false;

    /* Attribute names should be unique */
    if (!attr_names_unique(query->attr_names, query->attr_num))
        return false;

    /* Attributes should be present in relations listed */
    for (size_t attr_i = 0; attr_i < query->attr_num; attr_i++) {
        bool attr_found = false;
        for (size_t rel_i = 0; rel_i < query->rel_num; rel_i++) {
            relation_t *rel = catalogue_get_relation(cat, query->rel_names[rel_i]);
            if (!relation_has_attr(rel, query->attr_names[attr_i]))
                continue;
            attr_found = true;
            break;
        }
        if (attr_found)
            continue;

        const char *msg = "Error: unknown attribute name '%s'\n";
        fprintf(stderr, msg, query->attr_names[attr_i]);
        return false;
    }

    /* Order by attribute should be available in the list of attributes chosen */
    if (query->has_order) {
        if (!attr_in_attr_names(query->order_by_attr, query->attr_names, query->attr_num)) {
            const char *msg = "Error: unknown order by attribute '%s'\n";
            fprintf(stderr, msg, query->order_by_attr);
            return false;
        }
    }

    /* Predicate attributes should be available in the list of attributes projected */
    for (size_t pred_i = 0; pred_i < query->pred_num; pred_i++) {
        const query_predicate_t *predicate = &query->predicates[pred_i];

        /* Attribute on the left should always be there */
        {
            token_t token = predicate->left;
            char attr_name_buf[512] = {0};
            strncpy(attr_name_buf, token.start, (size_t)token.length);

            if (!attr_in_attr_names(attr_name_buf, query->attr_names, query->attr_num)) {
                const char *msg = "Error: unknown left-hand side attribute name '%s' in predicate %zu\n";
                fprintf(stderr, msg, attr_name_buf, pred_i);
                return false;
            }
        }

        /* Attribute on the right? */
        {
            token_t token = predicate->right;
            if (token.type == TOKEN_IDENT) {
                char attr_name_buf[512] = {0};
                strncpy(attr_name_buf, token.start, (size_t)token.length);

                if (!attr_in_attr_names(attr_name_buf, query->attr_names, query->attr_num)) {
                    const char *msg = "Error: unknown right-hand side attribute name '%s' in predicate %zu\n";
                    fprintf(stderr, msg, attr_name_buf, pred_i);
                    return false;
                }
            }
        }
    }

    return true;
}

static bool validate_create_table(catalogue_t *cat, const query_create_table_t *query)
{
    /* A relation should not exists */
    if (catalogue_get_relation(cat, query->rel_name)) {
        fprintf(stderr, "Error: relation '%s' already exists\n", query->rel_name);
        return false;
    }

    /* Attribute names are unique */
    if (!attr_names_unique(query->attr_names, query->attr_num))
        return false;

    return true;
}

static bool validate_insert(catalogue_t *cat, const query_insert_t *query)
{
    /* A relation should exists */
    relation_t *target_rel = catalogue_get_relation(cat, query->rel_name);
    if (!target_rel) {
        fprintf(stderr, "Error: relation '%s' does not exist\n", query->rel_name);
        return false;
    }

    /* Number of attribute values to be inserted should be correct */
    uint16_t rel_attr_num = relation_get_attr_num(target_rel);
    uint16_t query_attr_num = query->value_num;
    if (rel_attr_num != query_attr_num) {
        fprintf(stderr, "Error: relation '%s' has %"PRIu16" attributes, only %"PRIu16" supplied\n",
                query->rel_name, rel_attr_num, query_attr_num);
        return false;
    }

    return true;
}

bool validate(catalogue_t *cat, const query_t *query)
{
    switch (query->tag) {
    case QUERY_SELECT:
        return validate_select(cat, &query->as.select);
    case QUERY_CREATE_TABLE:
        return validate_create_table(cat, &query->as.create_table);
    case QUERY_INSERT:
        return validate_insert(cat, &query->as.insert);
    }
    assert(false);
}
