CC = gcc
CFLAGS = -std=gnu11 -O2 -g

all: pigletql

test: pigletql-eval-test pigletql-parser-test pigletql-catalogue-test
	./pigletql-eval-test
	./pigletql-parser-test
	./pigletql-catalogue-test

pigletql: pigletql.c pigletql-parser.c pigletql-eval.c pigletql-catalogue.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-catalogue-test: pigletql-catalogue-test.c pigletql-catalogue.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-eval-test: pigletql-eval-test.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

pigletql-parser-test: pigletql-parser-test.c pigletql-parser.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -vf pigletql pigletql-eval-test pigletql-parser-test pigletql-catalogue-test

.PHONY: all clean pigletql-eval-test pigletql-parser-test pigletql-catalogue-test
