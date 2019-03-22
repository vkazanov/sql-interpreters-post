CC = gcc
CFLAGS = -std=gnu11 -O2 -g

all: test

test: pigletql-eval-test
	./pigletql-eval-test

# pigletql: pigletql.c
# 	$(CC) $(CFLAGS) $^ -o $@

pigletql-eval-test: pigletql-eval-test.c pigletql-eval.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -vf pigletql pigletql-eval-test

.PHONY: all clean pigletql-test
