#include <stdlib.h>
#include <assert.h>

#include "pigletql-parser.h"
#include "pigletql-def.h"

typedef struct query_t {
    attr_name_t attr_names[MAX_ATTR_NUM];
    uint16_t attr_num;

    rel_name_t rel_names[MAX_REL_NUM];
    uint16_t rel_num;
} query_t;

typedef struct scanner_t {
    const char *string;
} scanner_t;

scanner_t *scanner_create(const char *string) {
    scanner_t *scanner = calloc(1, sizeof(*scanner));
    assert(scanner);

    scanner->string = string;

    return scanner;
}

void scanner_destroy(scanner_t *scanner) {
    if (scanner)
        free(scanner);
}

void parse(const char *query_string, query_t *query)
{
    (void) query_string; (void) query;

    scanner_t *scanner = scanner_create(query_string);

    scanner_destroy(scanner);
}
