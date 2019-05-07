#include <stdio.h>
#include <stdbool.h>

void run(char *line)
{
    (void) line;

    printf("%s", line);
    /* TODO: parse */
    /* TODO: dump the parse result */
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    char line[1024];

    while (true) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        run(line);
    }

    return 0;
}
