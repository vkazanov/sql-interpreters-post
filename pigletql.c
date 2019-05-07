#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pigletql-parser.h"
#include "pigletql-eval.h"

void eval(const query_t *query)
{
    (void) query;
    /* TODO: eval the parse result given some kind of state */
}

void run(const char *query_str)
{
    printf("%s\n", query_str);

    scanner_t *scanner = scanner_create(query_str);
    parser_t *parser = parser_create();
    query_t *query = query_create();

    if (parser_parse(parser, scanner, query))
        eval(query);

    scanner_destroy(scanner);
    parser_destroy(parser);
    query_destroy(query);
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    while (true) {
        char line[1024];

        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* strip a newline at the end of the line */
        line[strlen(line) - 1] = '\0';

        run(line);
    }

    return 0;
}
