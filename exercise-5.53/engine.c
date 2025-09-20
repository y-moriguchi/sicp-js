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
#include "memory.h"

static cell comp;
static cons *env;
static cell val;
static cont_type continuation;
static cell fun;
static cell argl;
static cell unev;
static cont_type next;

static char *symbol_of_name(cell c) {
    return check_and_get_symbol(head(tail(c)));
}

static int is_tagged_list(cell component, char *tag) {
    return is_pair(component) && equal_symbol(head(component), tag);
}

static int is_literal(cell component) {
    return is_tagged_list(component, "literal");
}

static cell literal_value(cell component) {
    return head(tail(component));
}

static cell make_function(cell parameters, cell body, cons *env) {
    return pair(get_symbol_len("compound_function"),
                pair(parameters,
                     pair(body,
                          pair(get_pointer(env),
                               get_nil()))));
}

static int is_compound_function(cell component) {
    return is_tagged_list(component, "compound_function");
}

static cell function_parameters(cell component) {
    return head(tail(component));
}

static cell function_body(cell component) {
    return head(tail(tail(component)));
}

static cons *function_environment(cell component) {
    return head(tail(tail(tail(component)))).datum.ptr;
}

static int is_continuation(cell component) {
    return is_tagged_list(component, "%cont");
}

static cell continuation_registers(cell component) {
    return head(tail(component));
}

static int is_application(cell component) {
    return is_tagged_list(component, "application");
}

static int is_unary_operator_combination(cell component) {
    return is_tagged_list(component, "unary_operator_combination");
}

static int is_operator_combination(cell component) {
    return is_unary_operator_combination(component) ||
           is_tagged_list(component, "binary_operator_combination");
}

static int is_lambda_expression(cell component) {
    return is_tagged_list(component, "lambda_expression");
}

static cell make_lambda_expression(cell params, cell body) {
    return pair(get_symbol_len("lambda_expression"),
                pair(params,
                     pair(body, get_nil())));
}

static int is_sequence(cell component) {
    return is_tagged_list(component, "sequence");
}

static cell sequence_statements(cell component) {
    return head(tail(component));
}

static int is_block(cell component) {
    return is_tagged_list(component, "block");
}

static cell block_body(cell component) {
    return head(tail(component));
}

static cell scan_out_declarations(cell component);

static cell list_of_unassigned(cell symbols) {
    return is_null(symbols)
           ? get_nil()
           : pair(get_symbol_len("*unassigned*"),
                  list_of_unassigned(tail(symbols)));
}

static int is_function_declaration(cell component) {
    return is_tagged_list(component, "function_declaration");
}

static cell function_declaration_name(cell component) {
    return head(tail(component));
}

static cell function_declaration_parameters(cell component) {
    return head(tail(tail(component)));
}

static cell function_declaration_body(cell component) {
    return head(tail(tail(tail(component))));
}

static int is_declaration(cell component) {
    return is_tagged_list(component, "constant_declaration") ||
           is_tagged_list(component, "variable_declaration") ||
           is_function_declaration(component);
}

static cell make_constant_declaration(cell name, cell value) {
    return pair(get_symbol_len("constant_declaration"),
                pair(name,
                     pair(value, get_nil())));
}

static cell function_decl_to_constant_decl(cell component) {
    return make_constant_declaration(
            function_declaration_name(component),
            make_lambda_expression(
                function_declaration_parameters(component),
                function_declaration_body(component)));
}

static char *declaration_symbol(cell component) {
    return symbol_of_name(head(tail(component)));
}

static cell declaration_value_expression(cell component) {
    return head(tail(tail(component)));
}

static cell map_statements(cell statements) {
    return is_null(statements)
           ? get_nil()
           : pair(scan_out_declarations(head(statements)),
                  map_statements(tail(statements)));
}

static cell accumlate_cells(cell cells) {
    return is_null(cells)
           ? get_nil()
           : append(head(cells), accumlate_cells(tail(cells)));
}

static cell accumlate_statements(cell statements) {
    return accumlate_cells(map_statements(statements));
}

static cell scan_out_declarations(cell component) {
    return is_sequence(component)
           ? accumlate_statements(sequence_statements(component))
           : is_declaration(component)
           ? pair(get_symbol_len(declaration_symbol(component)), get_nil())
           : get_nil();
}

static int is_return_statement(cell component) {
    return is_tagged_list(component, "return_statement");
}

static cell return_expression(cell component) {
    return head(tail(component));
}

static int is_name(cell component) {
    return is_tagged_list(component, "name");
}

static int is_conditional(cell component) {
    return is_tagged_list(component, "conditional_expression") ||
           is_tagged_list(component, "conditional_statement");
}

static cell conditional_predicate(cell component) {
    return head(tail(component));
}

static cell conditional_consequent(cell component) {
    return head(tail(tail(component)));
}

static cell conditional_alternative(cell component) {
    return head(tail(tail(tail(component))));
}

static int is_assignment(cell component) {
    return is_tagged_list(component, "assignment");
}

static char *assignment_symbol(cell component) {
    return symbol_of_name(head(tail(component)));
}

static cell assignment_value_expression(cell component) {
    return head(tail(tail(component)));
}

static cell first_statement(cell stmts) {
    return head(stmts);
}

static cell rest_statements(cell stmts) {
    return tail(stmts);
}

static char *operator_symbol(cell component) {
    return check_and_get_symbol(head(tail(component)));
}

static cell make_name(char *operator) {
    return pair(get_symbol_len("name"), pair(get_symbol_len(operator), get_nil()));
}

static cell first_operand(cell component) {
    return head(tail(tail(component)));
}

static cell second_operand(cell component) {
    return head(tail(tail(tail(component))));
}

static cell make_application1(cell op, cell first) {
    return pair(get_symbol_len("application"),
                pair(op, pair(pair(first, get_nil()), get_nil())));
}

static cell make_application2(cell op, cell first, cell second) {
    return pair(get_symbol_len("application"),
                pair(op, pair(pair(first, pair(second, get_nil())), get_nil())));
}

static cell operator_combination_to_application(cell component) {
    char *operator = operator_symbol(component);

    return is_unary_operator_combination(component)
           ? make_application1(make_name(operator),
                               first_operand(component))
           : make_application2(make_name(operator),
                               first_operand(component),
                               second_operand(component));
}

static int is_empty_sequence(cell stmts) {
    return is_null(stmts);
}

static int is_last_statement(cell stmts) {
    return is_null(tail(stmts));
}

static cell arg_expressions(cell component) {
    return head(tail(tail(component)));
}

static cell function_expression(cell component) {
    return head(tail(component));
}

static cell lambda_parameters(cell component) {
    return head(tail(component));
}

static cell lambda_parameter_symbols_inner(cell component) {
    return is_null(component)
           ? get_nil()
           : pair(get_symbol_len(symbol_of_name(head(component))),
                  lambda_parameter_symbols_inner(tail(component)));
}

static cell lambda_parameter_symbols(cell component) {
    return lambda_parameter_symbols_inner(lambda_parameters(component));
}

static cell lambda_body(cell component) {
    return head(tail(tail(component)));
}

static cell empty_arglist() {
    return get_nil();
}

static int is_last_argument_expression(cell arg_expression) {
    return is_null(tail(arg_expression));
}

static cell adjoin_arg(cell arg, cell arglist) {
    return append(arglist, pair(arg, get_nil()));
}

static void eval_dispatch();
static void apply_dispatch();

static void ev_conditional_decide() {
    continuation = restore_continuation();
    env = restore_cons();
    comp = restore();
    if(is_falsy(val)) {
        comp = conditional_alternative(comp);
        next = eval_dispatch;
    } else {
        comp = conditional_consequent(comp);
        next = eval_dispatch;
    }
}

static void ev_sequence_last_statement() {
    continuation = restore_continuation();
    next = eval_dispatch;
}

static void ev_sequence_next();

static void ev_sequence_continue() {
    env = restore_cons();
    unev = rest_statements(restore());
    next = ev_sequence_next;
}

static void ev_sequence_next() {
    comp = first_statement(unev);
    if(is_last_statement(unev)) {
        next = ev_sequence_last_statement;
    } else {
        save(unev);
        save_cons(env);
        continuation = ev_sequence_continue;
        next = eval_dispatch;
    }
}

static void ev_sequence_empty() {
    val = get_undefined();
    next = continuation;
}

static void ev_appl_argument_expression_loop();

static void ev_appl_accumulate_arg() {
    unev = restore();
    env = restore_cons();
    argl = adjoin_arg(val, restore());
    unev = tail(unev);
    next = ev_appl_argument_expression_loop;
}

static void ev_appl_accum_last_arg() {
    argl = adjoin_arg(val, restore());
    fun = restore();
    next = apply_dispatch;
}

static void ev_appl_last_arg() {
    continuation = ev_appl_accum_last_arg;
    next = eval_dispatch;
}

static void ev_appl_argument_expression_loop() {
    save(argl);
    comp = head(unev);
    if(is_last_argument_expression(unev)) {
        next = ev_appl_last_arg;
    } else {
        save_cons(env);
        save(unev);
        continuation = ev_appl_accumulate_arg;
        next = eval_dispatch;
    }
}

static void ev_appl_did_function_expression() {
    unev = restore();
    env = restore_cons();
    argl = empty_arglist();
    fun = val;
    if(is_null(unev)) {
        next = apply_dispatch;
    } else {
        save(fun);
        next = ev_appl_argument_expression_loop;
    }
}

static void ev_application() {
    save_continuation(continuation);
    save_cons(env);
    unev = arg_expressions(comp);
    save(unev);
    comp = function_expression(comp);
    continuation = ev_appl_did_function_expression;
    next = eval_dispatch;
}

static void ev_operator_combination() {
    comp = operator_combination_to_application(comp);
    next = ev_application;
}

static void primitive_apply() {
    val = apply_primitive_function(fun, argl);
    continuation = restore_continuation();
    next = continuation;
}

static void return_undefined() {
    revert_stack_to_marker();
    continuation = restore_continuation();
    val = get_undefined();
    next = continuation;
}

static void compound_apply() {
    unev = function_parameters(fun);
    env = extend_environment(unev, argl, function_environment(fun));
    comp = function_body(fun);
    push_marker_to_stack();
    continuation = return_undefined;
    next = eval_dispatch;
}

static void continuation_apply() {
    cell valtmp = head(argl);
    cons *regs = check_and_get_cons_ptr(continuation_registers(fun));

    restore_registers(regs);
    revert_stack_to_marker();
    continuation = restore_continuation();
    val = valtmp;
    next = continuation;
}

static void apply_dispatch() {
    if(is_primitive_function(fun)) {
        next = primitive_apply;
    } else if(is_compound_function(fun)) {
        next = compound_apply;
    } else if(is_continuation(fun)) {
        next = continuation_apply;
    } else {
        PUT_ERROR("Internal error -- apply_dispatch", get_nil());
    }
}

static void ev_return() {
    revert_stack_to_marker();
    continuation = restore_continuation();
    comp = return_expression(comp);
    next = eval_dispatch;
}

static void ev_block() {
    comp = block_body(comp);
    val = scan_out_declarations(comp);
    env = extend_environment(val, list_of_unassigned(val), env);
    next = eval_dispatch;
}

static void ev_assignment_install() {
    continuation = restore_continuation();
    env = restore_cons();
    unev = restore();
    assign_symbol_value(check_and_get_symbol(unev), val, env);
    next = continuation;
}

static void ev_assignment() {
    unev = get_symbol_len(assignment_symbol(comp));
    save(unev);
    comp = assignment_value_expression(comp);
    save_cons(env);
    save_continuation(continuation);
    continuation = ev_assignment_install;
    next = eval_dispatch;
}

static void ev_declaration_assign() {
    continuation = restore_continuation();
    env = restore_cons();
    unev = restore();
    assign_symbol_value(check_and_get_symbol(unev), val, env);
    val = get_undefined();
    next = continuation;
}

static void ev_declaration() {
    unev = get_symbol_len(declaration_symbol(comp));
    save(unev);
    comp = declaration_value_expression(comp);
    save_cons(env);
    save_continuation(continuation);
    continuation = ev_declaration_assign;
    next = eval_dispatch;
}

static void ev_function_declaration() {
    comp = function_decl_to_constant_decl(comp);
    next = ev_declaration;
}

static int is_logical_composition(cell component) {
    return is_tagged_list(component, "logical_composition");
}

static int is_logical_symbol(cell component, char *sym) {
    return equal_symbol(head(tail(component)), sym);
}

static cell logical_first_operand(cell component) {
    return head(tail(tail(component)));
}

static cell logical_second_operand(cell component) {
    return head(tail(tail(tail(component))));
}

static void and_first() {
    continuation = restore_continuation();
    comp = restore();
    if(is_falsy(val)) {
        next = continuation;
    } else {
        comp = logical_second_operand(comp);
        next = eval_dispatch;
    }
}

static void ev_and_composition() {
    save(comp);
    save_continuation(continuation);
    comp = logical_first_operand(comp);
    continuation = and_first;
    next = eval_dispatch;
}

static void or_first() {
    continuation = restore_continuation();
    comp = restore();
    if(is_falsy(val)) {
        comp = logical_second_operand(comp);
        next = eval_dispatch;
    } else {
        next = continuation;
    }
}

static void ev_or_composition() {
    save(comp);
    save_continuation(continuation);
    comp = logical_first_operand(comp);
    continuation = or_first;
    next = eval_dispatch;
}

static void eval_dispatch() {
    if(is_literal(comp)) {
        val = literal_value(comp);
        next = continuation;
    } else if(is_name(comp)) {
        val = lookup_variable_value(symbol_of_name(comp), env);
        next = continuation;
    } else if(is_application(comp)) {
        next = ev_application;
    } else if(is_operator_combination(comp)) {
        next = ev_operator_combination;
    } else if(is_logical_composition(comp)) {
        next = is_logical_symbol(comp, "&&") ? ev_and_composition : ev_or_composition;
    } else if(is_conditional(comp)) {
        save(comp);
        save_cons(env);
        save_continuation(continuation);
        continuation = ev_conditional_decide;
        comp = conditional_predicate(comp);
        next = eval_dispatch;
    } else if(is_lambda_expression(comp)) {
        unev = lambda_parameter_symbols(comp);
        comp = lambda_body(comp);
        val = make_function(unev, comp, env);
        next = continuation;
    } else if(is_sequence(comp)) {
        unev = sequence_statements(comp);
        if(is_empty_sequence(unev)) {
            next = ev_sequence_empty;
        } else {
            save_continuation(continuation);
            next = ev_sequence_next;
        }
    } else if(is_block(comp)) {
        next = ev_block;
    } else if(is_return_statement(comp)) {
        next = ev_return;
    } else if(is_function_declaration(comp)) {
        next = ev_function_declaration;
    } else if(is_declaration(comp)) {
        next = ev_declaration;
    } else if(is_assignment(comp)) {
        next = ev_assignment;
    } else {
        PUT_ERROR("unknown type -- eval_dispatch", head(comp));
    }
}

#define BUF 4000000

static char parser1[BUF];
static cons *parser_env;

static void read_parser() {
    int i, ch;
    FILE *file;

    if((file = fopen("jsparser.txt", "r")) == NULL) {
        perror("error");
        exit(4);
    } else {
        for(i = 0; i < BUF - 1 && (ch = getc(file)) != EOF; i++) {
            parser1[i] = ch;
        }
        parser1[i] = '\0';
        fclose(file);
    }
}

extern cons *create_environment(cell program, cons *environment) {
    cell tmpcomp;

    comp = program;
    val = scan_out_declarations(comp);
    tmpcomp = list_of_unassigned(val);
    env = extend_environment(val, tmpcomp, environment);
    next = eval_dispatch;
    continuation = NULL;
    save_continuation(continuation);
    while(next != NULL) {
        next();
    }
    return env;
}

static cell parse_js(char *program, cons *environment) {
    static char drive_parse[100000];
    cell result;

    if(strlen(program) > 99000) {
        PUT_ERROR("too long program -- parse_js", get_nil());
    } else {
        sprintf(drive_parse, "['application',[['name',['parse',null]],[[['literal',[\034%s\034,null]],null],null]]]", program);
        result = evaluate(parse(drive_parse), environment);
        return result;
    }
}

extern cell evaluate(cell program, cons *environment) {
    cell tmpcomp;

    comp = program;
    val = scan_out_declarations(comp);
    tmpcomp = list_of_unassigned(val);
    env = extend_environment(val, tmpcomp, environment);
    next = eval_dispatch;
    continuation = NULL;
    save_continuation(continuation);
    while(next != NULL) {
        next();
    }
    return val;
}

extern cell execute(char *program) {
    return evaluate(parse_js(program, parser_env), env);
}

static cell push_comp() {
    return comp;
}

static cell push_env() {
    return get_pointer(env);
}

static cell push_val() {
    return val;
}

static cell push_continuation() {
    return get_continuation(continuation);
}

static cell push_fun() {
    return fun;
}

static cell push_argl() {
    return argl;
}

static cell push_unev() {
    return unev;
}

static void relocate_comp(cell c) {
    comp = c;
}

static void relocate_env(cell c) {
    env = check_and_get_cons_ptr(c);
}

static void relocate_val(cell c) {
    val = c;
}

static void relocate_continuation(cell c) {
    continuation = check_and_get_continuation(c);
}

static void relocate_fun(cell c) {
    fun = c;
}

static void relocate_argl(cell c) {
    argl = c;
}

static void relocate_unev(cell c) {
    unev = c;
}

static cell save_parser_env() {
    return get_pointer(parser_env);
}

static void restore_parser_env(cell c) {
    parser_env = check_and_get_cons_ptr(c);
}

#define CALL_CC \
    "['sequence',[[['constant_declaration',[['name',['call_cc',null]]," \
    "[['lambda_expression',[[['name',['k',null]],null],[['block',[['sequence'," \
    "[[['constant_declaration',[['name',['a',null]],[['application',[['name'," \
    "['%getcont',null]],[null,null]]],null]]],[['return_statement'," \
    "[['application',[['name',['k',null]],[[['name',['a',null]],null],null]]]" \
    ",null]],null]],null]],null]],null]]],null]]],null],null]]"

extern void init_cons() {
    cons *env_local = setup_environment();

    cell cc = parse(CALL_CC);
    env = create_environment(cc, env_local);

    add_register(push_comp, relocate_comp);
    add_register(push_env, relocate_env);
    add_register(push_val, relocate_val);
    add_register(push_fun, relocate_fun);
    add_register(push_argl, relocate_argl);
    add_register(push_unev, relocate_unev);

    read_parser();
    parser_env = create_environment(parse(parser1), env);
    add_register(save_parser_env, restore_parser_env);
}

