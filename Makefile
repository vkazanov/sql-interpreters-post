CC = gcc
CFLAGS = -std=gnu11 -O2 -g

TESTS = pigletql-eval-test pigletql-parser-test pigletql-catalogue-test pigletql-validate-test

all: pigletql

test: $(TESTS)
	$(foreach interpr,$(TESTS),./$(interpr);)

pigletql: pigletql.c pigletql-parser.c pigletql-eval.c pigletql-catalogue.c pigletql-validate.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-catalogue-test: pigletql-catalogue-test.c pigletql-catalogue.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-eval-test: pigletql-eval-test.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-parser-test: pigletql-parser-test.c pigletql-parser.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-validate-test: pigletql-validate-test.c pigletql-parser.c pigletql-catalogue.c pigletql-validate.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -vf pigletql $(TESTS)

.PHONY: all clean $(TESTS)
