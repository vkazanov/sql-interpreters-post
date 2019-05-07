#ifndef PIGLETQL_DEF_H
#define PIGLETQL_DEF_H

#include <stdint.h>

/*
 * Common definitions
 *  */

/* maximum number of attributes per relation */
#define MAX_ATTR_NUM (UINT16_MAX - 1)
/* return value telling that an attribute was not found */
#define ATTR_NOT_FOUND UINT16_MAX
/* maximum attribute name length */
#define MAX_ATTR_NAME_LEN 256

/* maximum number of relations */
#define MAX_REL_NUM UINT16_MAX
/* maximum relation name length */
#define MAX_REL_NAME_LEN 256

/* maximum number of predicates per query */
#define MAX_PRED_NUM UINT16_MAX

typedef enum sort_order_t {
    SORT_ASC = 0,
    SORT_DESC,
} sort_order_t;

typedef uint32_t value_type_t;  /* a single value type supported */
typedef char attr_name_t[MAX_ATTR_NAME_LEN]; /* attribute names are fixed-size strings */
typedef char rel_name_t[MAX_REL_NAME_LEN]; /* relation names are fixed-size strings */

#endif //PIGLETQL_DEF_H
