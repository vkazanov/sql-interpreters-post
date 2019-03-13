#ifndef PIGLETQL_H
#define PIGLETQL_H

typedef struct operator_t operator_t;
typedef struct operator_state_t operator_state_t;

typedef struct tuple_proxy_t tuple_proxy_t;
typedef struct tuple_source_t tuple_source_t;
typedef enum tuple_tag tuple_tag;

typedef void (*operator_open)(operator_state_t *state);
typedef tuple_proxy_t *(*operator_next)(operator_state_t *state);
typedef void (*operator_close)(operator_state_t *state);

struct operator_state_t {
    operator_t *left_input;
    operator_t *right_input;
    void *state;
};

struct operator_t {
    operator_open open;
    operator_next next;
    operator_close close;
    operator_state_t *state;
};

enum tuple_tag {
    TUPLE_SOURCE,
    TUPLE_PROXY
};

struct tuple_proxy_t {
};

struct tuple_source_t {
};

struct tuple {
    tuple_tag tag;
    union {
        tuple_source_t source;
        tuple_proxy_t proxy;
    } as;
};

#endif //PIGLETQL_H
