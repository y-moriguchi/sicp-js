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
#include <math.h>
#include "memory.h"

static cell list(cell args) {
    return args;
}

static cell reverse_inner(cell list, cell reversed) {
    return is_null(list)
           ? reversed
           : reverse_inner(tail(list), pair(head(list), reversed));
}

static cell reverse(cell list) {
    return reverse_inner(head(list), get_nil());
}

static cell memv_inner(cell obj, cell list) {
    return is_null(list)
           ? get_false()
           : eqv(obj, head(list))
           ? list
           : memv_inner(obj, tail(list));
}

static cell memv(cell args) {
    return memv_inner(head(args), head(tail(args)));
}

static cell assv_inner(cell obj, cell list) {
    return is_null(list)
           ? get_false()
           : eqv(obj, head(head(list)))
           ? head(list)
           : assv_inner(obj, tail(list));
}

static cell assv(cell args) {
    return assv_inner(head(args), head(tail(args)));
}

extern cell append(cell cell1, cell cell2) {
    return is_null(cell1)
           ? cell2
           : pair(head(cell1), append(tail(cell1), cell2));
}

static cell append_args(cell args) {
    return append(head(args), head(tail(args)));
}

static cell negate(cell args) {
    cell right = head(args);
    cell result;

    if(right.type == NUMBER) {
        result.type = NUMBER;
        result.datum.number = -right.datum.number;
        return result;
    } else {
        PUT_ERROR("Invalid argument -- negate", get_nil());
    }
}

static cell add(cell args) {
    cell left = head(args);
    cell right = head(tail(args));
    cell result;

    if(left.type != right.type) {
        printf("%d %d\n", left.type, right.type);
        if(right.type == NUMBER) {
            printf("%lf\n", right.datum.number);
        }
        PUT_ERROR("Invalid argument -- add", get_nil());
    } else if(left.type == NUMBER) {
        result.type = NUMBER;
        result.datum.number = left.datum.number + right.datum.number;
        return result;
    } else if(left.type == SYMBOL) {
        PUT_ERROR("Not implemented -- add", get_nil());
    } else {
        PUT_ERROR("Invalid argument -- add", get_nil());
    }
}

static cell subtract(cell args) {
    cell left = head(args);
    cell right = head(tail(args));
    cell result;

    if(left.type != right.type) {
        PUT_ERROR("Invalid argument -- subtract", get_nil());
    } else if(left.type == NUMBER) {
        result.type = NUMBER;
        result.datum.number = left.datum.number - right.datum.number;
        return result;
    } else {
        PUT_ERROR("Invalid argument -- subtract", get_nil());
    }
}

static cell multiply(cell args) {
    cell left = head(args);
    cell right = head(tail(args));
    cell result;

    if(left.type != right.type) {
        PUT_ERROR("Invalid argument -- multiply", get_nil());
    } else if(left.type == NUMBER) {
        result.type = NUMBER;
        result.datum.number = left.datum.number * right.datum.number;
        return result;
    } else {
        PUT_ERROR("Invalid argument -- multiply", get_nil());
    }
}

static cell remainder_cell(cell args) {
    cell left = head(args);
    cell right = head(tail(args));
    cell result;

    if(left.type != right.type) {
        PUT_ERROR("Invalid argument -- remainder", get_nil());
    } else if(left.type == NUMBER) {
        result.type = NUMBER;
        result.datum.number = fmod(left.datum.number, right.datum.number);
        return result;
    } else {
        PUT_ERROR("Invalid argument -- remainder", get_nil());
    }
}

static int less_than(int type) {
    return type < 0;
}

static int less_than_equal(int type) {
    return type <= 0;
}

static int more_than(int type) {
    return type > 0;
}

static int more_than_equal(int type) {
    return type >= 0;
}

static cell compare_less_than(cell args) {
    return compare(head(args), head(tail(args)), less_than);
}

static cell compare_less_than_equal(cell args) {
    return compare(head(args), head(tail(args)), less_than_equal);
}

static cell compare_more_than(cell args) {
    return compare(head(args), head(tail(args)), more_than);
}

static cell compare_more_than_equal(cell args) {
    return compare(head(args), head(tail(args)), more_than_equal);
}

static cell logical_not(cell args) {
    return is_falsy(head(args)) ? get_true() : get_false();
}

static cell eqv_cell(cell args) {
    cell r = get_number(eqv(head(args), head(tail(args))));

    return r;
}

static cell not_eqv_cell(cell args) {
    return get_number(!eqv(head(args), head(tail(args))));
}

static cell pair_cell(cell args) {
    return pair(head(args), head(tail(args)));
}

static cell head_cell(cell args) {
    return head(head(args));
}

static cell tail_cell(cell args) {
    return tail(head(args));
}

static cell is_null_cell(cell args) {
    cell test = head(args);

    return is_null(test) ? get_true() : get_false();
}

static cell set_head_cell(cell args) {
    return set_head(head(args), head(tail(args)));
}

static cell set_tail_cell(cell args) {
    return set_tail(head(args), head(tail(args)));
}

static cell array_to_list(cell *ptr) {
    if(is_null(*ptr)) {
        return get_nil();
    } else {
        return pair(*ptr, array_to_list(ptr + 1));
    }
}

extern cell string_ref_cell(cell args) {
    return string_ref(head(args), check_and_get_int(head(tail(args))));
}

extern cell string_length_cell(cell args) {
    cell r = string_length(head(args));

    return r;
}

extern cell string_append_cell(cell args) {
    return string_append(head(args), head(tail(args)));
}

extern cell substring_cell(cell args) {
    int begin = check_and_get_int(head(tail(args)));
    int end = check_and_get_int(head(tail(tail(args))));
    cell r = substring(head(args), begin, end);

    return r;
}

extern cell char_to_integer_cell(cell args) {
    return get_number(char_to_integer(head(args)));
}

extern cell is_whitespace_cell(cell args) {
    return is_whitespace(head(args)) ? get_true() : get_false();
}

extern cell is_alphabetic_cell(cell args) {
    return is_alphabetic(head(args)) ? get_true() : get_false();
}

extern cell is_numeric_cell(cell args) {
    return is_numeric(head(args)) ? get_true() : get_false();
}

static cell display_cell(cell args) {
    display(head(args));
    return get_undefined();
}

static cell getcont_cell(cell args) {
    cons *cont = save_registers();

    return pair(get_symbol_len("%cont"), pair(get_pointer(cont), get_nil()));
}

static cell error_cell(cell error) {
    PUT_ERROR("Error", head(error));
}

static cell display_memory_usage_cell(cell args) {
    display_memory_usage();
    return get_undefined();
}

static cell *init_symbol_list() {
    static cell symbol_list[1000];
    cell *ptr = symbol_list;

    *ptr++ = get_symbol_len("pair");
    *ptr++ = get_symbol_len("head");
    *ptr++ = get_symbol_len("tail");
    *ptr++ = get_symbol_len("is_null");
    *ptr++ = get_symbol_len("list");
    *ptr++ = get_symbol_len("append");
    *ptr++ = get_symbol_len("reverse");
    *ptr++ = get_symbol_len("memv");
    *ptr++ = get_symbol_len("assv");
    *ptr++ = get_symbol_len("set_head");
    *ptr++ = get_symbol_len("set_tail");
    *ptr++ = get_symbol_len("string_ref");
    *ptr++ = get_symbol_len("string_length");
    *ptr++ = get_symbol_len("string_append");
    *ptr++ = get_symbol_len("substring");
    *ptr++ = get_symbol_len("char_to_integer");
    *ptr++ = get_symbol_len("is_whitespace");
    *ptr++ = get_symbol_len("is_alphabetic");
    *ptr++ = get_symbol_len("is_numeric");
    *ptr++ = get_symbol_len("===");
    *ptr++ = get_symbol_len("!==");
    *ptr++ = get_symbol_len("<");
    *ptr++ = get_symbol_len("<=");
    *ptr++ = get_symbol_len(">");
    *ptr++ = get_symbol_len(">=");
    *ptr++ = get_symbol_len("!");
    *ptr++ = get_symbol_len("+");
    *ptr++ = get_symbol_len("-");
    *ptr++ = get_symbol_len("*");
    *ptr++ = get_symbol_len("unary-");
    *ptr++ = get_symbol_len("%");
    *ptr++ = get_symbol_len("%getcont");
    *ptr++ = get_symbol_len("error");
    *ptr++ = get_symbol_len("display");
    *ptr++ = get_symbol_len("display_memory_usage");
    *ptr++ = get_nil();
    return symbol_list;
}

static cell *init_primitive_list() {
    static cell primitive_list[1000];
    cell *ptr = primitive_list;

    *ptr++ = get_primitive(pair_cell);
    *ptr++ = get_primitive(head_cell);
    *ptr++ = get_primitive(tail_cell);
    *ptr++ = get_primitive(is_null_cell);
    *ptr++ = get_primitive(list);
    *ptr++ = get_primitive(append_args);
    *ptr++ = get_primitive(reverse);
    *ptr++ = get_primitive(memv);
    *ptr++ = get_primitive(assv);
    *ptr++ = get_primitive(set_head_cell);
    *ptr++ = get_primitive(set_tail_cell);
    *ptr++ = get_primitive(string_ref_cell);
    *ptr++ = get_primitive(string_length_cell);
    *ptr++ = get_primitive(string_append_cell);
    *ptr++ = get_primitive(substring_cell);
    *ptr++ = get_primitive(char_to_integer_cell);
    *ptr++ = get_primitive(is_whitespace_cell);
    *ptr++ = get_primitive(is_alphabetic_cell);
    *ptr++ = get_primitive(is_numeric_cell);
    *ptr++ = get_primitive(eqv_cell);
    *ptr++ = get_primitive(not_eqv_cell);
    *ptr++ = get_primitive(compare_less_than);
    *ptr++ = get_primitive(compare_less_than_equal);
    *ptr++ = get_primitive(compare_more_than);
    *ptr++ = get_primitive(compare_more_than_equal);
    *ptr++ = get_primitive(logical_not);
    *ptr++ = get_primitive(add);
    *ptr++ = get_primitive(subtract);
    *ptr++ = get_primitive(multiply);
    *ptr++ = get_primitive(negate);
    *ptr++ = get_primitive(remainder_cell);
    *ptr++ = get_primitive(getcont_cell);
    *ptr++ = get_primitive(error_cell);
    *ptr++ = get_primitive(display_cell);
    *ptr++ = get_primitive(display_memory_usage_cell);
    *ptr++ = get_nil();
    return primitive_list;
}

extern cons *setup_environment() {
    return extend_environment(
            array_to_list(init_symbol_list()),
            array_to_list(init_primitive_list()),
            NULL);
}

