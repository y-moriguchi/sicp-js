/*
 * Solution of SICP JS Exercise 5.53
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
#define TRUE 1
#define FALSE 0
#define SHORT_LENGTH 7

#define PUT_ERROR(msg, obj) { put_error(msg, obj); exit(10); }

enum code {
    POINTER,
    NUMBER,
    SYMBOL,
    SHORT_SYMBOL,
    PRIMITIVE,
    CONTINUATION,
    TRUE_LITERAL,
    FALSE_LITERAL,
    UNDEFINED,
    NONE,
    MOVED,
    MARKER
};

struct cons_tag;
struct cell_tag;

typedef void (*cont_type)();

typedef struct cell_tag {
    enum code type;
    union cell_inner_tag {
        struct cons_tag *ptr;
        double number;
        char *symbol;
        char short_symbol[SHORT_LENGTH + 1];
        struct cell_tag (*primitive)(struct cell_tag);
        cont_type cont;
    } datum;
} cell;

typedef struct cons_tag {
    cell head_cell;
    cell tail_cell;
} cons;

typedef cell (*push_register)();
typedef void (*relocate_register)(cell);

extern cons *save_registers();
extern void restore_registers(cons *root_new);
extern void add_register(push_register, relocate_register);
extern cons *alloc_cell(cell head, cell tail);
extern cell get_pointer(cons *ptr);
extern cell get_nil();
extern cell get_symbol(char *, int);
extern cell get_symbol_len(char *);
extern int equal_symbol(cell c, char *sym);
extern cell compare(cell left, cell right, int (*compare_type)(int));
extern cell get_true();
extern cell get_false();
extern cell get_none();
extern cell get_undefined();
extern cell get_number(double number);
extern cell get_continuation(cont_type);
extern cell get_none();
extern cell get_primitive(cell (*primitive)(cell));
extern int is_null(cell);
extern int is_pair(cell);
extern int is_none(cell);
extern int is_primitive_function(cell);
extern int is_falsy(cell);
extern int is_truthy(cell);
extern cell pair(cell, cell);
extern cell head(cell);
extern cell tail(cell);
extern cell set_head(cell, cell);
extern cell set_tail(cell, cell);
extern cell lookup_variable_value(char *sym, cons *env);
extern void assign_symbol_value(char *sym, cell val, cons *env);
extern cons *extend_environment(cell unev, cell argl, cons *env);
extern void save(cell);
extern void save_cons(cons *);
extern void save_continuation(cont_type);
extern cell restore();
extern cons *restore_cons();
extern cont_type restore_continuation();
extern void push_marker_to_stack();
extern void revert_stack_to_marker();
extern cell apply_primitive_function(cell fun, cell argl);
extern cons *setup_environment();
extern cell append(cell, cell);
extern cell parse(char *prog);
extern cell evaluate(cell prog, cons *env);
extern cons *create_environment(cell program, cons *environment);
extern char *check_and_get_symbol(cell c);
extern double check_and_get_number(cell c);
extern int check_and_get_int(cell c);
extern cons *check_and_get_cons_ptr(cell c);
extern cont_type check_and_get_continuation(cell c);
extern cell string_ref(cell c, int i);
extern cell string_length(cell c);
extern cell string_append(cell s1, cell s2);
extern cell substring(cell s, int begin, int end);
extern int char_to_integer(cell s);
extern int is_whitespace(cell s);
extern int is_alphabetic(cell s);
extern int is_numeric(cell s);
extern int eqv(cell c1, cell c2);
extern void display(cell to_display);
extern cell execute(char *program);
extern void init_cons();
extern void display_memory_usage();
extern void put_error(char *msg, cell obj);
extern void init_memory();

