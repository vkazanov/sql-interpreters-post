CC = gcc
CFLAGS = -std=gnu11 -O3 -g

all: test

test: pigletql-test

# pigletql: pigletql.c
# 	$(CC) $(CFLAGS) $^ -o $@

pigletql-test: pigletql-test.c pigletql.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -vf pigletql pigletql-test

.PHONY: all clean pigletql-test
