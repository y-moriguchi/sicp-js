/*
 * Solution of SICP JS Exercise 5.53
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define MAX_NUMBER_STRING 501

static char *program;
static char *current;

static void init_parser(char *prog) {
    program = current = prog;
}

enum {
    LIST, STRING_LITERAL, NUMBER_LITERAL, NULL_LITERAL, BOOL_LITERAL
};

enum {
    FIRST, MINUS, NUM
};

typedef cell (*parser)(char *cur);

static double string_to_number(char *src, int len) {
    static char buf[MAX_NUMBER_STRING];
    double result = -1;

    if(len >= MAX_NUMBER_STRING - 1) {
        PUT_ERROR("Number too long -- string_to_number", get_nil());
    }
    strncpy(buf, src, len);
    buf[len] = '\0';
    if(sscanf(buf, "%lf", &result) < 1) {
        PUT_ERROR("Cannot convert number -- string_to_number", get_nil());
    }
    return result;
}

static cell parse_string_1(char *cur, char ch);
static cell parse_string_2(char *cur, char ch);
static cell parse_string_3(char *cur, char ch);

static cell parse_string_1(char *cur, char ch) {
    if(*cur == ch) {
        return parse_string_2(cur + 1, ch);
    } else {
        return get_none();
    }
}

static cell parse_string_2(char *cur, char ch) {
    cell result;

    if(*cur == '\0') {
        PUT_ERROR("Invalid string -- parse_string_2", get_nil());
    } else if(*cur == ch) {
        result = get_symbol(current + 1, cur - current - 1);
        current = cur + 1;
        return result;
//    } else if(*cur == '\\') {
//        return parse_string_3(cur + 1, ch);
    } else {
        return parse_string_2(cur + 1, ch);
    }
}

static cell parse_string_3(char *cur, char ch) {
    if(*cur == '\0') {
        PUT_ERROR("Invalid string -- parse_string_3", get_nil());
    } else {
        return parse_string_2(cur + 1, ch);
    }
}

static cell parse_string_dquote(char *cur) {
    return parse_string_1(cur, '\"');
}

static cell parse_string_quote(char *cur) {
    return parse_string_1(cur, '\'');
}

static cell parse_string_subsep(char *cur) {
    return parse_string_1(cur, '\034');
}

static cell parse_number_1(char *cur, int first);
static cell parse_number_2(char *cur);
static cell parse_number_3(char *cur);
static cell parse_number_4(char *cur, int first);
static cell parse_number_5(char *cur);

static cell parse_number_1(char *cur, int first) {
    cell result;

    if(isdigit(*cur)) {
        return parse_number_1(cur + 1, NUM);
    } else if(*cur == '.') {
        return parse_number_2(cur + 1);
    } else if(*cur == 'e' || *cur == 'E') {
        return parse_number_4(cur + 1, TRUE);
    } else if(*cur == '-') {
        if(first == FIRST) {
            return parse_number_1(cur + 1, MINUS);
        } else if(first == MINUS) {
            return get_none();
        } else if(first == NUM) {
            result = get_number(string_to_number(current, cur - current));
            current = cur;
            return result;
        } else {
            PUT_ERROR("Internal Error -- parse_number_1", get_nil());
        }
    } else {
        if(first == NUM) {
            result = get_number(string_to_number(current, cur - current));
            current = cur;
            return result;
        } else {
            return get_none();
        }
    }
}

static cell parse_number_2(char *cur) {
    if(isdigit(*cur)) {
        return parse_number_3(cur + 1);
    } else {
        PUT_ERROR("Invalid number -- parse_number_2", get_nil());
    }
}

static cell parse_number_3(char *cur) {
    cell result;

    if(isdigit(*cur)) {
        return parse_number_3(cur + 1);
    } else if(*cur == 'e' || *cur == 'E') {
        return parse_number_4(cur + 1, TRUE);
    } else {
        result = get_number(string_to_number(current, cur - current));
        current = cur;
        return result;
    } 
}

static cell parse_number_4(char *cur, int first) {
    if(isdigit(*cur)) {
        return parse_number_5(cur + 1);
    } else if(*cur == '-') {
        if(first) {
            return parse_number_4(cur + 1, FALSE);
        } else {
            PUT_ERROR("Invalid number -- parse_number_4", get_nil());
        }
    } else {
        PUT_ERROR("Invalid number -- parse_number_4", get_nil());
    }
}

static cell parse_number_5(char *cur) {
    cell result;

    if(isdigit(*cur)) {
        return parse_number_5(cur + 1);
    } else {
        result = get_number(string_to_number(current, cur - current));
        current = cur;
        return result;
    }
}

static cell parse_number(char *cur) {
    return parse_number_1(cur, FIRST);
}

static cell start_with_word(char *cur, char *str2, cell token) {
    if(*cur == '\0') {
        if(*str2 == '\0') {
            current = cur;
            return token;
        } else {
            return get_none();
        }
    } else if(*str2 == '\0') {
        if(isalnum(*cur) || *cur == '_' || *cur == '$') {
            return get_none();
        } else {
            current = cur;
            return token;
        }
    } else if(*cur == *str2) {
        return start_with_word(cur + 1, str2 + 1, token);
    } else {
        return get_none();
    }
}

static cell parse_null(char *cur) { return start_with_word(cur, "null", get_nil()); }
static cell parse_true(char *cur) { return start_with_word(cur, "true", get_true()); }
static cell parse_false(char *cur) { return start_with_word(cur, "false", get_false()); }

static parser parsers[] = {
    parse_number,
    parse_string_dquote,
    parse_string_quote,
    parse_string_subsep,
    parse_null,
    parse_true,
    parse_false,
    NULL
};

static cell parse_primitive() {
    parser *now;
    cell result;

    if(*current == '\0') {
        return get_none();
    } else {
        for(now = parsers; *now != NULL; now++) {
            result = (*now)(current);
            if(result.type != NONE) {
                return result;
            }
        }
        PUT_ERROR("Lexer error -- parse_primitive", get_nil());
    }
}

static char *skip_space_1(char *cur) {
    if(isspace(*cur)) {
        return skip_space_1(cur + 1);
    } else {
        return cur;
    }
}

static void skip_space() {
    current = skip_space_1(current);
}

static cell parse_inner() {
    cell head, tail;

    skip_space();
    if(*current == '\0') {
        PUT_ERROR("Unexpected EOF -- parse_inner", get_nil());
    } else if(*current == '[') {
        current++;
        skip_space();
        head = parse_inner();
        skip_space();
        if(*current != ',') {
            printf("%c\n", *current);
            PUT_ERROR("Invalid pair: , -- parse_inner", get_nil());
        }
        current++;
        skip_space();
        tail = parse_inner();
        skip_space();
        if(*current != ']') {
            PUT_ERROR("Invalid pair: ] -- parse_inner", get_nil());
        }
        current++;
        return get_pointer(alloc_cell(head, tail));
    } else {
        return parse_primitive();
    }
}

extern cell parse(char *prog) {
    cell result;

    init_parser(prog);
    result = parse_inner();
    skip_space();
    if(*current == '\0') {
        return result;
    } else {
        PUT_ERROR("Syntax error -- parse", get_nil());
    }
}

/*int main(int argc, char **argv) {
    parse("[-11.23e-3, [2, \"aaa\"]]");
}*/

