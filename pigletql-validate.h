#ifndef PIGLETQL_VALIDATE_H
#define PIGLETQL_VALIDATE_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "pigletql-catalogue.h"
#include "pigletql-parser.h"

bool validate(catalogue_t *cat, const query_t *query);

#endif //PIGLETQL-VALIDATE_H
