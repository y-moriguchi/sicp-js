/*
 * Solution of SICP JS Exercise 5.53
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
%token RETURN CONST LET FUNCTION IF ELSE
%token NULL_WORD TRUE_WORD FALSE_WORD ARROW START_ARROW
%token <num> NUMBER_WORD
%token <str> STRING_LITERAL
%token <datum> NAME NAME_ARROW

%type <datum> sequence sequence_list statement conditional_statement return_statement constant_declaration variable_declaration
%type <datum> function_declaration block names name_list
%type <datum> expression expression_op function_expression element expressions expression_list lambda_expression

%left OR
%left AND
%left EQ NE
%left '<' LE '>' GE
%left '+' '-'
%left '*' '/' '%'
%nonassoc '!' UMINUS 

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"

static cell final_result;

void yyerror(const char *s);
int yylex(void);
%}

%union {
  char *str;
  double num;
  cell datum;
}

%%

program : sequence { final_result = $1; }

sequence: sequence_list { $$ = make_sequence($1); }

sequence_list : /* empty */        { $$ = get_nil(); }
              | statement sequence_list { $$ = pair($1, $2); }

statement : expression ';'          { $$ = $1; }
          | NAME '=' expression ';' { $$ = make_assignment($1, $3); }
          | conditional_statement
          | return_statement
          | constant_declaration
          | variable_declaration
          | function_declaration

conditional_statement : IF '(' expression ')' block ELSE block
                        { $$ = make_conditional("conditional_statement", $3, $5, $7); }
                      | IF '(' expression ')' block ELSE conditional_statement
                        { $$ = make_conditional("conditional_statement", $3, $5, $7); }

return_statement : RETURN expression ';' { $$ = make_return_statement($2); }

constant_declaration : CONST NAME '=' expression ';' { $$ = make_constant_declaration($2, $4); }

variable_declaration : LET NAME '=' expression ';'   { $$ = make_variable_declaration($2, $4); }

function_declaration : FUNCTION NAME '(' names ')' block { $$ = make_function_declaration($2, $4, $6); }

block : '{' sequence '}' { $$ = make_block($2); }

names : /* empty */ { $$ = get_nil(); }
      | name_list

name_list : NAME               { $$ = pair($1, get_nil()); }
          | NAME ',' name_list { $$ = pair($1, $3); }

expression: expression_op
          | expression_op '?' expression_op ':' expression
            { $$ = make_conditional("conditional_expression", $1, $3, $5); }
          | lambda_expression

expression_op : expression_op OR expression_op  { $$ = make_binary_operator_combination("||", $1, $3); }
              | expression_op AND expression_op { $$ = make_binary_operator_combination("&&", $1, $3); }
              | expression_op EQ expression_op  { $$ = make_binary_operator_combination("===", $1, $3); }
              | expression_op NE expression_op  { $$ = make_binary_operator_combination("!==", $1, $3); }
              | expression_op '>' expression_op { $$ = make_binary_operator_combination(">", $1, $3); }
              | expression_op GE expression_op  { $$ = make_binary_operator_combination(">=", $1, $3); }
              | expression_op '<' expression_op { $$ = make_binary_operator_combination("<", $1, $3); }
              | expression_op LE expression_op  { $$ = make_binary_operator_combination("<=", $1, $3); }
              | expression_op '+' expression_op { $$ = make_binary_operator_combination("+", $1, $3); }
              | expression_op '-' expression_op { $$ = make_binary_operator_combination("-", $1, $3); }
              | expression_op '*' expression_op { $$ = make_binary_operator_combination("*", $1, $3); }
              | expression_op '/' expression_op { $$ = make_binary_operator_combination("/", $1, $3); }
              | expression_op '%' expression_op { $$ = make_binary_operator_combination("%", $1, $3); }
              | '-' expression_op %prec UMINUS  { $$ = make_unary_operator_combination("unary-", $2); }
              | '!' expression_op               { $$ = make_unary_operator_combination("!", $2); }
              | function_expression

function_expression : function_expression '(' expressions ')' { $$ = make_application($1, $3); }
                    | element

element : '(' expression ')' { $$ = $2; }
        | NUMBER_WORD    { $$ = make_literal(get_number($1)); }
        | STRING_LITERAL { $$ = make_literal(get_symbol_len($1)); }
        | NULL_WORD      { $$ = make_literal(get_nil()); }
        | TRUE_WORD      { $$ = make_literal(get_true()); }
        | FALSE_WORD     { $$ = make_literal(get_false()); }
        | NAME

expressions : /* empty */     { $$ = get_nil(); }
            | expression_list

expression_list : expression                     { $$ = pair($1, get_nil()); }
                | expression ',' expression_list { $$ = pair($1, $3); }

lambda_expression : START_ARROW names ')' ARROW block { $$ = make_lambda_expression($2, $5); }
                  | START_ARROW names ')' ARROW expression
                    { $$ = make_lambda_expression($2, make_block(make_sequence(pair(make_return_statement($5), get_nil())))); }
                  | NAME_ARROW ARROW block    { $$ = make_lambda_expression(pair($1, get_nil()), $3); }
                  | NAME_ARROW ARROW expression
                    { $$ = make_lambda_expression(pair($1, get_nil()), make_block(make_sequence(pair(make_return_statement($3), get_nil())))); }

%%
#define NOT_MATCHED 0
#define ARENA_SIZE 200000

static char arena_base[ARENA_SIZE + 1];
static char *arena_ptr = arena_base;

static void *alloc_arena(size_t size) {
  if(arena_ptr - arena_base + size >= ARENA_SIZE) {
    PUT_ERROR("Out of memory -- arena", get_nil());
  } else {
    void *result = arena_ptr;

    arena_ptr += size;
    return result;
  }
}

static char *newstr_arena(char *str, size_t len) {
  char *result = alloc_arena(len + 1);

  strncpy(result, str, len);
  result[len] = '\0';
  return result;
}

static char *program;
static char *current;

extern void init_parser(char *prog) {
  program = current = prog;
}

enum {
  FIRST, MINUS, NUM
};

typedef int (*lexer)(char *cur);

static int lex_symbol_1(char *cur);
static int lex_symbol_2(char *cur);
static int lex_symbol_3(char *cur);

static int lex_symbol_1(char *cur) {
  if(isalpha(*cur) || *cur == '_' || *cur == '$') {
    return lex_symbol_2(cur + 1);
  } else {
    return NOT_MATCHED;
  }
}

static int lex_symbol_2(char *cur) {
  if(isalnum(*cur) || *cur == '_' || *cur == '$') {
    return lex_symbol_2(cur + 1);
  } else if(lex_symbol_3(cur)) {
    yylval.datum = make_name(newstr_arena(current, cur - current));
    current = cur;
    return NAME_ARROW;
  } else {
    yylval.datum = make_name(newstr_arena(current, cur - current));
    current = cur;
    return NAME;
  }
}

static int lex_symbol_3(char *cur) {
  if(*cur == ' ' || *cur == '\t' || *cur == '\n') {
    return lex_symbol_3(cur + 1);
  } else if(*cur == '=') {
    return cur[1] == '>';
  } else {
    return FALSE;
  }
}

static int lex_string_1(char *cur, char ch);
static int lex_string_2(char *cur, char ch);
static int lex_string_3(char *cur, char ch);

static int lex_string_1(char *cur, char ch) {
  if(*cur == ch) {
    return lex_string_2(cur + 1, ch);
  } else {
    return NOT_MATCHED;
  }
}

static int lex_string_2(char *cur, char ch) {
  if(*cur == '\0') {
    perror("Invalid string");
    exit(4);
  } else if(*cur == ch) {
    yylval.str = newstr_arena(current, cur - current);
    current = cur;
    return STRING_LITERAL;
  } else if(*cur == '\\') {
    return lex_string_3(cur + 1, ch);
  } else {
    return lex_string_2(cur + 1, ch);
  }
}

static int lex_string_3(char *cur, char ch) {
  if(*cur == '\0') {
    perror("Invalid string");
    exit(4);
  } else {
    return lex_string_2(cur + 1, ch);
  }
}

static int lex_string_quote(char *cur) {
  return lex_string_1(cur, '\'');
}

static int lex_string_dquote(char *cur) {
  return lex_string_1(cur, '\"');
}

static int lex_string_bquote(char *cur) {
  return lex_string_1(cur, '`');
}

static int lex_number_1(char *cur, int first);
static int lex_number_2(char *cur);
static int lex_number_3(char *cur);
static int lex_number_4(char *cur);
static int lex_number_5(char *cur);

static int lex_number_1(char *cur, int first) {
  if(isdigit(*cur)) {
    return lex_number_1(cur + 1, NUM);
  } else if(*cur == '.') {
    return lex_number_2(cur + 1);
  } else if(*cur == 'e' || *cur == 'E') {
    return lex_number_4(cur + 1);
  } else if(*cur == '-') {
    if(first == FIRST) {
      return lex_number_1(cur + 1, MINUS);
    } else if(first == MINUS) {
      return NOT_MATCHED;
    } else if(first == NUM) {
      yylval.num = atof(newstr_arena(current, cur - current));
      current = cur;
      return NUMBER_WORD;
    } else {
      perror("Internal Error");
      exit(4);
    }
  } else {
    if(first == NUM) {
      yylval.num = atof(newstr_arena(current, cur - current));
      current = cur;
      return NUMBER_WORD;
    } else {
      return NOT_MATCHED;
    }
  }
}

static int lex_number_2(char *cur) {
  if(isdigit(*cur)) {
    return lex_number_3(cur + 1);
  } else {
    perror("Invalid number");
    exit(4);
  }
}

static int lex_number_3(char *cur) {
  if(isdigit(*cur)) {
    return lex_number_3(cur + 1);
  } else if(*cur == 'e' || *cur == 'E') {
    return lex_number_4(cur + 1);
  } else {
    yylval.num = atof(newstr_arena(current, cur - current));
    current = cur;
    return NUMBER_WORD;
  } 
}

static int lex_number_4(char *cur) {
  if(isdigit(*cur)) {
    return lex_number_5(cur + 1);
  } else {
    perror("Invalid number");
    exit(4);
  }
}

static int lex_number_5(char *cur) {
  if(isdigit(*cur)) {
    return lex_number_5(cur + 1);
  } else {
    yylval.num = atof(newstr_arena(current, cur - current));
    current = cur;
    return NUMBER_WORD;
  }
}

static int lex_number(char *cur) {
  return lex_number_1(cur, FIRST);
}

static int start_with(char *cur, char *str2, int token) {
  if(*str2 == '\0') {
    current = cur;
    return token;
  } else if(*cur == '\0' || *cur != *str2) {
    return NOT_MATCHED;
  } else {
    return start_with(cur + 1, str2 + 1, token);
  }
}

static int lex_eq(char *cur) { return start_with(cur, "===", EQ); }
static int lex_ne(char *cur) { return start_with(cur, "!==", NE); }
static int lex_ge(char *cur) { return start_with(cur, "<=", GE); }
static int lex_le(char *cur) { return start_with(cur, ">=", LE); }
static int lex_lambda(char *cur) { return start_with(cur, "=>", ARROW); }
static int lex_and(char *cur) { return start_with(cur, "&&", AND); }
static int lex_or(char *cur) { return start_with(cur, "||", OR); }

static int lex_start_arrow_2(char *cur) {
  if(cur[0] == '=' && cur[1] == '>') {
    return TRUE;
  } else if(*cur == ' ' || *cur == '\t' || *cur == '\n') {
    return lex_start_arrow_2(cur + 1);
  } else {
    return FALSE;
  }
}

static int lex_start_arrow_1(char *cur) {
  if(*cur == '\0') {
    return FALSE;
  } else if(*cur == ')') {
    return lex_start_arrow_2(cur + 1);
  } else {
    return lex_start_arrow_1(cur + 1);
  }
}

static int lex_start_arrow(char *cur) {
  if(*cur == '(' && lex_start_arrow_1(cur + 1)) {
    current = cur + 1;
    return START_ARROW;
  } else {
    return NOT_MATCHED;
  }
}

static int start_with_word(char *cur, char *str2, int token) {
  if(*cur == '\0') {
    if(*str2 == '\0') {
      current = cur;
      return token;
    } else {
      return NOT_MATCHED;
    }
  } else if(*str2 == '\0') {
    if(isalnum(cur[0]) || cur[0] == '_' || cur[0] == '$') {
      return NOT_MATCHED;
    } else {
      current = cur;
      return token;
    }
  } else if(*cur == *str2) {
    return start_with_word(cur + 1, str2 + 1, token);
  } else {
    return NOT_MATCHED;
  }
}

static int lex_if(char *cur) { return start_with_word(cur, "if", IF); }
static int lex_else(char *cur) { return start_with_word(cur, "else", ELSE); }
static int lex_return(char *cur) { return start_with_word(cur, "return", RETURN); }
static int lex_const(char *cur) { return start_with_word(cur, "const", CONST); }
static int lex_let(char *cur) { return start_with_word(cur, "let", LET); }
static int lex_function(char *cur) { return start_with_word(cur, "function", FUNCTION); }
static int lex_null(char *cur) { return start_with_word(cur, "null", NULL_WORD); }
static int lex_true(char *cur) { return start_with_word(cur, "true", TRUE_WORD); }
static int lex_false(char *cur) { return start_with_word(cur, "false", FALSE_WORD); }

static int lex_ch(char *cur) {
  int r = *cur;

  current = cur + 1;
  return r;
}

static void skip_space(char *cur) {
  if(*cur == ' ' || *cur == '\t' || *cur == '\n') {
    skip_space(cur + 1);
  } else {
    current = cur;
  }
}

static lexer lexers[] = {
  lex_string_quote,    // 0
  lex_string_dquote,
  lex_string_bquote,
  lex_eq,
  lex_ne,
  lex_ge,
  lex_le,
  lex_lambda,
  lex_start_arrow,
  lex_and,
  lex_or,              // 10
  lex_if,
  lex_else,
  lex_return,
  lex_const,
  lex_let,
  lex_function,
  lex_null,
  lex_true,
  lex_false,
  lex_symbol_1,        // 20
  lex_number,
  lex_ch,
  NULL
};

void yyerror(const char *s) {
    fprintf(stderr, "error: %s\n", s);
}

int yylex(void) {
  lexer *now;
  int result;

  if(*current == '\0') {
    return 0;
  } else {
    skip_space(current);
    for(now = lexers; now != NULL; now++) {
      if((result = (*now)(current)) != NOT_MATCHED) {
        skip_space(current);
        return result;
      }
    }
    perror("Lexer error");
    exit(4);
  }
}

extern cell parse_js_bison(char *program) {
  final_result = get_nil();
  init_parser(program);
  yyparse();
  return final_result;
}

