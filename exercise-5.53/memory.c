/*
 * Solution of SICP JS Exercise 5.53
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"

#define MEMORY_COLLECT_SIZE 10000000
#define MEMORY_SIZE (MEMORY_COLLECT_SIZE + 2000)
#define SYMBOL_MEMORY_SIZE 4000000
#define REGISTERS 200

static cons memory1[MEMORY_SIZE];
static cons memory2[MEMORY_SIZE];
static cell stack;

static char symbol_memory1[SYMBOL_MEMORY_SIZE];
static char symbol_memory2[SYMBOL_MEMORY_SIZE];

static long freep;
static long scanp;
static cons *root;
static cell old;
static cell newp;

static cons *the_memory = memory1;
static cons *new_memory = memory2;
static long symbol_freep = 0;
static char *the_symbol_memory = symbol_memory1;
static char *new_symbol_memory = symbol_memory2;

static push_register push_registers[REGISTERS];
static relocate_register relocate_registers[REGISTERS];
static int registers_count = 0;

static char *alloc_symbol(size_t size);

extern cell get_pointer(cons *ptr) {
    cell result;

    result.type = POINTER;
    result.datum.ptr = ptr;
    return result;
}

extern cell get_nil() {
    static cell nil;
    static int init = FALSE;

    if(!init) {
        init = TRUE;
        nil = get_pointer(NULL);
    }
    return nil;
}

extern cell get_symbol(char *src, int len) {
    cell result;

    if(len <= SHORT_LENGTH) {
        result.type = SHORT_SYMBOL;
        strncpy(result.datum.short_symbol, src, len);
        result.datum.short_symbol[len] = '\0';
    } else {
        result.type = SYMBOL;
        result.datum.symbol = alloc_symbol(len + 1);
        if(result.datum.symbol == NULL) {
            PUT_ERROR("Out of memory -- get_symbol", get_nil());
        } else {
            strncpy(result.datum.symbol, src, len);
            result.datum.symbol[len] = '\0';
        }
    }
    return result;
}

extern cell get_symbol_len(char *src) {
    return get_symbol(src, strlen(src));
}

extern int equal_symbol(cell c, char *sym) {
    if(c.type == SYMBOL) {
        return strcmp(c.datum.symbol, sym) == 0;
    } else if(c.type == SHORT_SYMBOL) {
        return strcmp(c.datum.short_symbol, sym) == 0;
    } else {
        PUT_ERROR("Not symbol -- equal_symbol", c);
    }
}

static int compare_number(double num1, double num2) {
    return num1 < num2 ? -1 : num1 > num2 ? 1 : 0;
}

extern cell compare(cell left, cell right, int (*compare_type)(int)) {
    if(left.type == SYMBOL && right.type == SHORT_SYMBOL) {
        return get_number(compare_type(strcmp(left.datum.symbol, right.datum.short_symbol)));
    } else if(left.type == SHORT_SYMBOL && right.type == SYMBOL) {
        return get_number(compare_type(strcmp(left.datum.short_symbol, right.datum.symbol)));
    } else if(left.type != right.type) {
        PUT_ERROR("Invalid argument compare -- compare_number", get_nil());
    } else if(left.type == NUMBER) {
        return get_number(compare_type(compare_number(left.datum.number, right.datum.number)));
    } else if(left.type == SYMBOL) {
        return get_number(compare_type(strcmp(left.datum.symbol, right.datum.symbol)));
    } else if(left.type == SHORT_SYMBOL) {
        return get_number(compare_type(strcmp(left.datum.short_symbol, right.datum.short_symbol)));
    } else {
        PUT_ERROR("Invalid argument compare -- compare_number", get_nil());
    }
}

extern cell get_true() {
    static cell trueval = { TRUE_LITERAL, { NULL } };

    return trueval;
}

extern cell get_false() {
    static cell falseval = { FALSE_LITERAL, { NULL } };

    return falseval;
}

extern cell get_undefined() {
    static cell undefval = { UNDEFINED, { NULL } };

    return undefval;
}

static cell get_marker() {
    static cell markerval = { MARKER, { NULL } };

    return markerval;
}

extern cell get_number(double number) {
    cell result;

    result.type = NUMBER;
    result.datum.number = number;
    return result;
}

extern cell get_continuation(cont_type cont) {
    cell result;

    result.type = CONTINUATION;
    result.datum.cont = cont;
    return result;
}

extern cell get_none() {
    cell result;

    result.type = NONE;
    return result;
}

extern cell get_primitive(cell (*primitive)(cell)) {
    cell result;

    result.type = PRIMITIVE;
    result.datum.primitive = primitive;
    return result;
}

extern char *check_and_get_symbol(cell c) {
    char *r;

    if(c.type == SYMBOL) {
        return c.datum.symbol;
    } else if(c.type == SHORT_SYMBOL) {
        r = alloc_symbol(strlen(c.datum.short_symbol) + 1);
        strcpy(r, c.datum.short_symbol);
        return r;
    } else {
        PUT_ERROR("Not symbol -- check_and_get_symbol", c);
    }
}

extern double check_and_get_number(cell c) {
    if(c.type != NUMBER) {
        PUT_ERROR("Not number -- check_and_get_number", c);
    } else {
        return c.datum.number;
    }
}

extern int check_and_get_int(cell c) {
    if(c.type != NUMBER) {
        PUT_ERROR("Not number -- check_and_get_number", c);
    } else {
        return (int)c.datum.number;
    }
}

extern cons *check_and_get_cons_ptr(cell c) {
    if(c.type != POINTER) {
        PUT_ERROR("Not pointer -- check_and_get_cons_ptr", c);
    } else {
        return c.datum.ptr;
    }
}

extern cont_type check_and_get_continuation(cell c) {
    if(c.type != CONTINUATION) {
        PUT_ERROR("Not continuation -- check_and_get_continuation", c);
    } else {
        return c.datum.cont;
    }
}

extern int is_null(cell c) {
    return c.type == POINTER && c.datum.ptr == NULL;
}

extern int is_pair(cell c) {
    return c.type == POINTER && c.datum.ptr != NULL;
}

extern int is_none(cell c) {
    return c.type == NONE;
}

extern int is_primitive_function(cell c) {
    return c.type == PRIMITIVE;
}

extern int is_undefined(cell c) {
    return c.type == UNDEFINED;
}

extern int is_falsy(cell c) {
    return c.type == FALSE_LITERAL ||
           is_null(c) ||
           (c.type == NUMBER && c.datum.number == 0) ||
           (c.type == SYMBOL && strlen(c.datum.symbol) == 0) ||
           (c.type == SHORT_SYMBOL && strlen(c.datum.short_symbol) == 0) ||
           c.type == UNDEFINED;
}

extern int is_truthy(cell c) {
    return !is_falsy(c);
}

extern cell pair(cell h, cell t) {
    return get_pointer(alloc_cell(h, t));
}

extern cell head(cell c) {
    if(is_pair(c)) {
        return c.datum.ptr->head_cell;
    } else {
        display(c);
        PUT_ERROR("Not pair -- head", c);
    }
}

extern cell tail(cell c) {
    if(is_pair(c)) {
        return c.datum.ptr->tail_cell;
    } else {
        display(c);
        PUT_ERROR("Not pair -- tail", c);
    }
}

extern cell set_head(cell c, cell val) {
    if(is_pair(c)) {
        c.datum.ptr->head_cell = val;
        return get_undefined();
    } else {
        PUT_ERROR("Not pair -- set_head", c);
    }
}

extern cell set_tail(cell c, cell val) {
    if(is_pair(c)) {
        c.datum.ptr->tail_cell = val;
        return get_undefined();
    } else {
        PUT_ERROR("Not pair -- set_tail", c);
    }
}

static void set_pointer(cell *cell, cons *ptr) {
    cell->type = POINTER;
    cell->datum.ptr = ptr;
}

static void relocate_old_result_in_new(int is_root) {
    cons *oldht;
    cell *ht;
    char *newsymbol;

    if(old.type == POINTER && old.datum.ptr != NULL) {
        oldht = old.datum.ptr;
        if(oldht->head_cell.type == MOVED) {
            newp = oldht->tail_cell;
        } else {
            set_pointer(&newp, new_memory + freep);
            freep++;
            if(freep >= MEMORY_COLLECT_SIZE) {
                printf("Out of memory -- cell\n");
                exit(10);
            }
            *newp.datum.ptr = *oldht;
            oldht->head_cell.type = MOVED;
            oldht->tail_cell = newp;
        }
    } else if(old.type == SYMBOL) {
        newp = old;
        newsymbol = new_symbol_memory + symbol_freep;
        symbol_freep += strlen(old.datum.symbol) + 1;
        if(symbol_freep >= SYMBOL_MEMORY_SIZE) {
            printf("Out of memory -- symbol_cell\n");
            exit(10);
        }
        strcpy(newsymbol, old.datum.symbol);
        newp.datum.symbol = newsymbol;
    } else {
        newp = old;
    }
}

extern void display_memory_usage() {
    printf("%ld/%d used\n", freep, MEMORY_COLLECT_SIZE);
}

static void gc_collect_inner(cons *root1) {
    cons *temp;
    char *symbol_temp;

    root = root1;
    freep = 0;
    scanp = 0;
    symbol_freep = 0;
    set_pointer(&old, root);
    relocate_old_result_in_new(0);
    root = newp.datum.ptr;
    while(scanp != freep) {
        old = new_memory[scanp].head_cell;
        relocate_old_result_in_new(1);
        new_memory[scanp].head_cell = newp;
        old = new_memory[scanp].tail_cell;
        relocate_old_result_in_new(2);
        new_memory[scanp].tail_cell = newp;
        scanp++;
    }
    temp = the_memory;
    the_memory = new_memory;
    new_memory = temp;
    symbol_temp = the_symbol_memory;
    the_symbol_memory = new_symbol_memory;
    new_symbol_memory = symbol_temp;
}

static cons *alloc_cell_register(cell head_cell, cons *tail_ptr) {
    int resultptr;

    the_memory[freep].head_cell = head_cell;
    the_memory[freep].tail_cell = get_pointer(tail_ptr);
    resultptr = freep++;
    if(freep >= MEMORY_SIZE) {
        PUT_ERROR("Internal error -- too many registers", get_nil());
    } else {
        return the_memory + resultptr;
    }
}

extern cons *save_registers() {
    cons *root_new = NULL;
    int i;

    root_new = alloc_cell_register(stack, root_new);
    for(i = 0; i < registers_count; i++) {
        root_new = alloc_cell_register(push_registers[i](), root_new);
    }
    return root_new;
}

extern void restore_registers(cons *root_new) {
    int i;

    for(i = registers_count - 1; i >= 0; i--) {
        relocate_registers[i](root_new->head_cell);
        if(is_pair(root_new->tail_cell)) {
            root_new = root_new->tail_cell.datum.ptr;
        } else {
            PUT_ERROR("Internal error -- bad register", get_nil());
        }
    }

    if(is_null(root_new->tail_cell)) {
        stack = root_new->head_cell;
    } else {
        PUT_ERROR("Internal error -- bad register", get_nil());
    }
}

static void gc_collect(cell *extra1, cell *extra2) {
    cons *root_new = NULL;

    root_new = save_registers();
    if(extra1 != NULL) {
        root_new = alloc_cell_register(*extra1, root_new);
    }
    if(extra2 != NULL) {
        root_new = alloc_cell_register(*extra2, root_new);
    }

    gc_collect_inner(root_new);

    root_new = root;
    if(extra2 != NULL) {
        *extra2 = root_new->head_cell;
        root_new = root_new->tail_cell.datum.ptr;
    }
    if(extra1 != NULL) {
        *extra1 = root_new->head_cell;
        root_new = root_new->tail_cell.datum.ptr;
    }
    restore_registers(root_new);
}

extern void add_register(push_register pusher, relocate_register relocater) {
    if(registers_count < REGISTERS) {
        push_registers[registers_count] = pusher;
        relocate_registers[registers_count] = relocater;
        registers_count++;
    } else {
        PUT_ERROR("Internal error -- too many listeners", get_nil());
    }
}

static cons *alloc_cell_inner(cell head_cell, cell tail_cell) {
    int resultptr;

    the_memory[freep].head_cell = head_cell;
    the_memory[freep].tail_cell = tail_cell;
    resultptr = freep++;
    if(freep >= MEMORY_COLLECT_SIZE) {
        return NULL;
    } else {
        return the_memory + resultptr;
    }
}

extern cons *alloc_cell(cell head_cell, cell tail_cell) {
    cons *result;

    if((result = alloc_cell_inner(head_cell, tail_cell)) != NULL) {
        return result;
    } else {
        gc_collect(&head_cell, &tail_cell);
        if((result = alloc_cell_inner(head_cell, tail_cell)) != NULL) {
            return result;
        } else {
            PUT_ERROR("Out of memory -- alloc_cell", get_nil());
        }
    }
}

static char *alloc_symbol_inner(size_t size) {
    char *result = the_symbol_memory + symbol_freep;

    if(symbol_freep + size < SYMBOL_MEMORY_SIZE) {
        symbol_freep += size;
        return result;
    } else {
        return NULL;
    }
}

static char *alloc_symbol(size_t size) {
    char *result;

    if((result = alloc_symbol_inner(size)) != NULL) {
        return result;
    } else {
        gc_collect(NULL, NULL);
        if((result = alloc_symbol_inner(size)) != NULL) {
            return result;
        } else {
            PUT_ERROR("Out of memory -- alloc_symbol", get_nil());
        }
    }
}

extern void save(cell to_push) {
    stack = pair(to_push, stack);
}

extern void save_cons(cons *to_push) {
    save(get_pointer(to_push));
}

extern void save_continuation(cont_type to_push) {
    save(get_continuation(to_push));
}

static void check_not_empty() {
    if(is_null(stack)) {
        PUT_ERROR("stack underflow -- check_not_empty", get_nil());
    }
}

extern cell restore() {
    cell result = head(stack);

    stack = tail(stack);
    return result;
}

extern cons *restore_cons() {
    return check_and_get_cons_ptr(restore());
}

extern cont_type restore_continuation() {
    return check_and_get_continuation(restore());
}

extern void push_marker_to_stack() {
    save(get_marker());
}

extern void revert_stack_to_marker() {
    cell r;

    while((r = restore()).type != MARKER) {
    }
}

static cell lookup_variable_value1(char *sym, cell env) {
    if(is_null(env)) {
        return get_none();
    } else if(!is_pair(head(env))) {
        PUT_ERROR("Internal error -- lookup_variable_value1", head(env));
    } else if(head(head(env)).type != SYMBOL && head(head(env)).type != SHORT_SYMBOL) {
        PUT_ERROR("Not symbol -- lookup_variable_value1", head(head(env)));
    } else if(equal_symbol(head(head(env)), sym)) {
        return tail(head(env));
    } else {
        return lookup_variable_value1(sym, tail(env));
    }
}

extern cell lookup_variable_value_inner(char *sym, cell env) {
    cell r;

    if(is_null(env)) {
        PUT_ERROR("Unbound symbol -- lookup_variable_value_inner", env);
    } else if(!is_pair(head(env))) {
        return lookup_variable_value_inner(sym, tail(env));
    } else {
        r = lookup_variable_value1(sym, head(env));
        return r.type == NONE ? lookup_variable_value_inner(sym, tail(env)) : r;
    }
}

extern cell lookup_variable_value(char *sym, cons *env) {
    return lookup_variable_value_inner(sym, get_pointer(env));
}

static int assign_symbol_value1(char *sym, cell val, cell env) {
    if(is_null(env)) {
        return FALSE;
    } else if(!is_pair(env)) {
        PUT_ERROR("Internal error -- assign_symbol_value1", env);
    } else if(!is_pair(head(env))) {
        PUT_ERROR("Internal error -- assign_symbol_value1", head(env));
    } else if(head(head(env)).type != SYMBOL && head(head(env)).type != SHORT_SYMBOL) {
        PUT_ERROR("Internal error -- assign_symbol_value1", head(head(env)));
    } else if(equal_symbol(head(head(env)), sym)) {
        set_tail(head(env), val);
        return TRUE;
    } else {
        return assign_symbol_value1(sym, val, tail(env));
    }
}

extern void assign_symbol_value_inner(char *sym, cell val, cell env) {
    if(is_null(env)) {
        PUT_ERROR("Unbound symbol -- assign_symbol_value_inner", env);
    } else if(!is_pair(env)) {
        PUT_ERROR("Internal error -- assign_symbol_value_inner", env);
    } else if(!is_pair(head(env))) {
        assign_symbol_value_inner(sym, val, tail(env));
    } else if(!assign_symbol_value1(sym, val, head(env))) {
        assign_symbol_value_inner(sym, val, tail(env));
    } else {
        // assigned
    }
}

extern void assign_symbol_value(char *sym, cell val, cons *env) {
    assign_symbol_value_inner(sym, val, get_pointer(env));
}

static cell int_env(cell unev, cell argl) {
    cell r;

    if(is_null(unev) && is_null(argl)) {
        return get_nil();
    } else if(is_pair(unev) && is_pair(argl)) {
        r = int_env(tail(unev), tail(argl));
        return pair(pair(head(unev), head(argl)), r);
    } else {
        PUT_ERROR("Internal error -- int_env", get_nil());
    }
}

extern cons *extend_environment(cell unev, cell argl, cons *env) {
    return alloc_cell(int_env(unev, argl), get_pointer(env));
}

extern cell apply_primitive_function(cell fun, cell argl) {
    if(fun.type != PRIMITIVE) {
        display(head(fun));
        PUT_ERROR("Not primitive -- apply_primitive_function", fun);
    }
    return fun.datum.primitive(argl);
}

extern cell string_ref(cell c, int i) {
    char *sym = check_and_get_symbol(c);

    if(strlen(sym) <= i) {
        PUT_ERROR("Length too short -- string_ref", get_nil());
    } else {
        return get_symbol(sym + i, 1);
    }
}

extern cell string_length(cell c) {
    char *sym = check_and_get_symbol(c);

    return get_number(strlen(sym));
}

extern cell string_append(cell s1, cell s2) {
    char *sym1 = check_and_get_symbol(s1);
    char *sym2 = check_and_get_symbol(s2);
    int len = strlen(sym1) + strlen(sym2);
    char *tmp = malloc(len + 1);
    cell result;

    sprintf(tmp, "%s%s", sym1, sym2);
    result = get_symbol(tmp, len);
    free(tmp);
    return result;
}

extern cell substring(cell s, int begin, int end) {
    char *sym = check_and_get_symbol(s);
    int len = strlen(sym);

    if(len <= begin || begin > end) {
        PUT_ERROR("Invalid range -- substring", get_nil());
    } else {
        return get_symbol(sym + begin, end - begin);
    }
}

extern int char_to_integer(cell s) {
    char *sym = check_and_get_symbol(s);

    if(strlen(sym) <= 0) {
        PUT_ERROR("Length too short -- char_to_integer", s);
    } else {
        return sym[0];
    }
}

extern int is_whitespace(cell s) {
    char *sym = check_and_get_symbol(s);

    if(strlen(sym) < 1) {
        PUT_ERROR("Length too short -- is_whitespace", s);
    } else {
        return isspace(*sym);
    }
}

extern int is_alphabetic(cell s) {
    char *sym = check_and_get_symbol(s);

    if(strlen(sym) < 1) {
        PUT_ERROR("Length too short -- is_alphabetic", s);
    } else {
        return isalpha(*sym);
    }
}

extern int is_numeric(cell s) {
    char *sym = check_and_get_symbol(s);

    if(strlen(sym) < 1) {
        PUT_ERROR("Length too short -- is_numeric", s);
    } else {
        return isdigit(*sym);
    }
}

extern int eqv(cell c1, cell c2) {
    if(c1.type == SYMBOL && c2.type == SHORT_SYMBOL) {
        return strcmp(c1.datum.symbol, c2.datum.short_symbol) == 0;
    } else if(c1.type == SHORT_SYMBOL && c2.type == SYMBOL) {
        return strcmp(c1.datum.short_symbol, c2.datum.symbol) == 0;
    } else if(c1.type != c2.type) {
        return FALSE;
    } else if(c1.type == POINTER) {
        return c1.datum.ptr == c2.datum.ptr;
    } else if(c1.type == NUMBER) {
        return c1.datum.number == c2.datum.number;
    } else if(c1.type == SYMBOL) {
        return strcmp(c1.datum.symbol, c2.datum.symbol) == 0;
    } else if(c1.type == SHORT_SYMBOL) {
        return strcmp(c1.datum.short_symbol, c2.datum.short_symbol) == 0;
    } else if(c1.type == PRIMITIVE) {
        return c1.datum.primitive == c2.datum.primitive;
    } else if(c1.type == TRUE_LITERAL) {
        return TRUE;
    } else if(c1.type == FALSE_LITERAL) {
        return TRUE;
    } else if(c1.type == UNDEFINED) {
        return TRUE;
    } else {
        return TRUE;
    }
}

static void display_inner(cell to_display) {
    if(is_pair(to_display)) {
        printf("[");
        display_inner(head(to_display));
        printf(", ");
        display_inner(tail(to_display));
        printf("]");
    } else if(is_null(to_display)) {
        printf("null");
    } else if(to_display.type == NUMBER) {
        printf("%lf", to_display.datum.number);
    } else if(to_display.type == SYMBOL) {
        printf("%s", to_display.datum.symbol);
    } else if(to_display.type == SHORT_SYMBOL) {
        printf("%s", to_display.datum.short_symbol);
    } else if(to_display.type == PRIMITIVE) {
        printf("<primitive>");
    } else if(to_display.type == CONTINUATION) {
        printf("<cont>");
    } else if(to_display.type == TRUE_LITERAL) {
        printf("true");
    } else if(to_display.type == FALSE_LITERAL) {
        printf("false");
    } else if(to_display.type == UNDEFINED) {
        printf("undefined");
    } else {
        printf("<other>");
    }
}

extern void display(cell to_display) {
    display_inner(to_display);
    printf("\n");
}

static char *display_error(cell to_display) {
    static char buf[1000];

    if(is_pair(to_display)) {
        return "<pair>";
    } else if(is_null(to_display)) {
        return "null";
    } else if(to_display.type == NUMBER) {
        sprintf(buf, "%lf", to_display.datum.number);
        return buf;
    } else if(to_display.type == SYMBOL) {
        sprintf(buf, "%s", to_display.datum.symbol);
        return buf;
    } else if(to_display.type == SHORT_SYMBOL) {
        sprintf(buf, "%s", to_display.datum.short_symbol);
        return buf;
    } else if(to_display.type == PRIMITIVE) {
        return "<primitive>";
    } else if(to_display.type == CONTINUATION) {
        return "<cont>";
    } else if(to_display.type == TRUE_LITERAL) {
        return "true";
    } else if(to_display.type == FALSE_LITERAL) {
        return "false";
    } else if(to_display.type == UNDEFINED) {
        return "undefined";
    } else {
        return "<other>";
    }
}

extern void put_error(char *msg, cell obj) {
    if(is_null(obj)) {
        fprintf(stderr, "%s\n", msg);
    } else {
        fprintf(stderr, "%s: %s\n", msg, display_error(obj));
    }
}

extern void init_memory() {
    stack = get_nil();
}

