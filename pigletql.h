#ifndef PIGLETQL_H
#define PIGLETQL_H

typedef struct operator operator;
typedef struct operator_state operator_state;

typedef struct record_proxy record_proxy;
typedef struct record_source record_source;
typedef enum record_tag record_tag;

typedef void (*operator_open)(operator_state *state);
typedef record_proxy *(*operator_next)(operator_state *state);
typedef void (*operator_close)(operator_state *state);

struct operator_state {
    operator *left_input;
    operator *right_input;
    void *state;
};

struct operator {
    operator_open open;
    operator_next next;
    operator_close close;
    operator_state *state;
};

enum record_tag {
    RECORD_SOURCE,
    RECORD_PROXY
};

struct record_proxy {
};

struct record_source {
};

struct record {
    record_tag tag;
    union {
        record_source source;
        record_proxy proxy;
    } as;
};

#endif //PIGLETQL_H
