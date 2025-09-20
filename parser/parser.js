/*
 * Parser of SICP JS
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
const tokenize_string = delim => {
    const tokenize_string_1 = (s, i) => {
        return i >= string_length(s)
               ? null
               : string_ref(s, i) === delim
               ? tokenize_string_2(s, i + 1, "")
               : null;
    };
    const tokenize_string_2 = (s, i, str) => {
        if(i >= string_length(s)) {
           error("invalid string");
        } else {
            const ch = string_ref(s, i);

            return ch === delim
                   ? make_token("literal", i + 1, str)
                   : ch === "\\"
                   ? tokenize_string_3(s, i + 1, string_append(str, ch))
                   : tokenize_string_2(s, i + 1, string_append(str, ch));
        }
    };
    const tokenize_string_3 = (s, i, str) => {
        return i >= string_length(s)
               ? error("invalid string")
               : tokenize_string_2(s, i + 1, string_append(str, string_ref(s, i)));
    };
    return tokenize_string_1;
};

const reserved_words = list(
    pair("null",     list("literal", null)),
    pair("true",     list("literal", true)),
    pair("false",    list("literal", false)),
    pair("function", list("function", false)),
    pair("return",   list("return", "return")),
    pair("const",    list("const", "const")),
    pair("let",      list("let", "let")),
    pair("if",       list("if", "if")),
    pair("else",     list("else", "else")));

const get_reserved = (i, sym) => {
    const reserved = assv(sym, reserved_words);

    return reserved !== false
           ? make_token(head(tail(reserved)), i, head(tail(tail(reserved))))
           : make_token("symbol", i, sym);
};

const tokenize_symbol_1 = (s, i) => {
    if(i >= string_length(s)) {
        return null;
    } else {
        const ch = string_ref(s, i);

        return is_alphabetic(ch) || ch === "_" || ch === "$"
               ? tokenize_symbol_2(s, i + 1, ch)
               : null;
    }
};

const tokenize_symbol_2 = (s, i, sym) => {
    if(i >= string_length(s)) {
        return make_token("symbol", i, sym);
    } else {
        const ch = string_ref(s, i);

        return is_alphabetic(ch) || is_numeric(ch) || ch === "_" || ch === "$"
               ? tokenize_symbol_2(s, i + 1, string_append(sym, ch))
               : get_reserved(i, sym);
    }
};

const tokenize_number_1 = (s, i) => {
    return tokenize_number_2(s, i, true);
};

const tokenize_number_2 = (s, i, start) => {
    if(i >= string_length(s)) {
        return null;
    } else if(string_ref(s, i) === "-") {
        if(start) {
            const val = tokenize_number_2(s, i + 1, false);

            return is_null(val)
                   ? null
                   : make_token("literal", get_token_index(val), -get_token(val));
        } else {
            return null;
        }
    } else {
        const ch = string_ref(s, i);
        const mantissa = is_numeric(ch)
                         ? tokenize_number_3(s, i, 0)
                         : ch === "."
                         ? tokenize_number_4(s, i + 1, 0)
                         : null;

        if(is_null(mantissa)) {
            return null;
        } else {
            const che = string_ref(s, head(mantissa));

            if(che === "e" || che === "E") {
                const exponent = tokenize_number_5(s, head(mantissa) + 1, true);

                return make_token("literal",
                    head(exponent),
                    tail(mantissa) * pow_int(10, tail(exponent)));
            } else {
                return make_token("literal", head(mantissa), tail(mantissa));
            }
        }
    }
};

const tokenize_number_3 = (s, i, num) => {
    if(i >= string_length(s)) {
        return pair(i, num);
    } else {
        const ch = string_ref(s, i);

        if(ch === ".") {
            const decimal = tokenize_number_4(s, i + 1);

            return pair(head(decimal), num + tail(decimal) / 10);
        } else {
            return is_numeric(ch)
                   ? tokenize_number_3(s, i + 1, num * 10 + char_to_digit(ch))
                   : pair(i, num);
        }
    }
};

const tokenize_number_4 = (s, i) => {
    if(i >= string_length(s)) {
        return 0;
    } else {
        const ch = string_ref(s, i);

        if(is_numeric(ch)) {
            const num = tokenize_number_4(s, i + 1);

            return pair(head(num), char_to_digit(ch) + tail(num) / 10);
        } else {
            return pair(i, 0);
        }
    }
};

const tokenize_number_5 = (s, i, start) => {
    if(i >= string_length(s)) {
        error("Syntax error: tokenize");
    } else if(string_ref(s, i) === "-") {
        if(start) {
            const num = tokenize_number_5(s, i + 1, false);

            return pair(head(num), -tail(num));
        } else {
            return error("Syntax error: tokenize");
        }
    } else {
        const ch = string_ref(s, i);

        return is_numeric(ch)
               ? tokenize_number_6(s, i + 1, char_to_digit(ch))
               : error("Syntax error: tokenize");
    }
};

const tokenize_number_6 = (s, i, num) => {
    if(i >= string_length(s)) {
        return pair(i, num);
    } else {
        const ch = string_ref(s, i);

        return is_numeric(ch)
               ? tokenize_number_6(s, i + 1, num * 10 + char_to_digit(ch))
               : pair(i, num);
    }
};

const charcode_0 = char_to_integer("0");
const char_to_digit = ch => char_to_integer(ch) - charcode_0;

const string_start_with = (type, dest) => {
    return (s, i) => {
        const dest_len = string_length(dest);

        return i + dest_len > string_length(s)
               ? null
               : substring(s, i, i + dest_len) === dest
               ? make_token(type, i + dest_len, dest)
               : null;
    };
};

const string_symbol_literal = (matcher, value) => (s, i) => {
    const result = matcher(s, i);

    return is_null(result) ? null : make_token("literal", get_token_index(result), value);
};

const string_symbol = matcher => (s, i) => {
    const result = matcher(s, i);

    return is_null(result)
           ? null
           : make_token(get_token_type(result), get_token_index(result), get_token_type(result));
};

const skip_space = (s, i) => {
    return i >= string_length(s)
           ? i
           : is_whitespace(string_ref(s, i))
           ? skip_space(s, i + 1)
           : i;
};

const tokenize_list = list(
    tokenize_string("\""),
    tokenize_string("'"),
    tokenize_string("`"),
    tokenize_number_1,
    string_start_with("binary_operator", "==="),
    string_start_with("binary_operator", "!=="),
    string_start_with("binary_operator", "<="),
    string_start_with("binary_operator", ">="),
    string_symbol(string_start_with("=>", "=>")),
    string_start_with("logical_composition", "&&"),
    string_start_with("logical_composition", "||"),
    string_symbol(string_start_with(";", ";")),
    string_symbol(string_start_with("(", "(")),
    string_symbol(string_start_with(")", ")")),
    string_symbol(string_start_with("{", "{")),
    string_symbol(string_start_with("}", "}")),
    string_symbol(string_start_with(",", ",")),
    string_symbol(string_start_with("=", "=")),
    string_start_with("binary_operator", "+"),
    string_start_with("binary_operator", "-"),
    string_start_with("binary_operator", "*"),
    string_start_with("binary_operator", "/"),
    string_start_with("binary_operator", "%"),
    string_start_with("binary_operator", "<"),
    string_start_with("binary_operator", ">"),
    string_symbol(string_start_with("?", "?")),
    string_symbol(string_start_with(":", ":")),
    string_start_with("unary_operator", "!"),
    tokenize_symbol_1
);

const tokenize = (s, i, tokens) => {
    const tokenize_symbol = (s, first_index, tokenizers) => {
        if(first_index >= string_length(s)) {
            return null;
        } else if(is_null(tokenizers)) {
            error("Syntax error: tokenize");
        } else {
            const result = head(tokenizers)(s, first_index);

            return is_null(result)
                   ? tokenize_symbol(s, first_index, tail(tokenizers))
                   : result;
        }
    };
    const first_index = skip_space(s, i);

    if(first_index >= string_length(s)) {
        return reverse(tokens);
    } else {
        const token = tokenize_symbol(s, first_index, tokenize_list);

        return is_null(token)
               ? null
               : tokenize(s,
                   get_token_index(token),
                   pair(remove_index(token), tokens));
    }
};

const make_token = (type, index, token) => list(type, token, index);
const get_token_type = token => is_null(token) ? null : head(token);
const get_token = token => is_null(token) ? null : head(tail(token));
const get_token_index = token => is_null(token) ? null : head(tail(tail(token)));
const remove_index = token => list(head(token), head(tail(token)));

const make_parser_result = (ast, tokens) => pair(ast, tokens);
const get_ast_result = result => head(result);
const get_tokens_result = result => tail(result);

const literal_expression = tokens => {
    const token = head(tokens);

    return get_token_type(token) === "literal"
           ? make_parser_result(
               list("literal", get_token(token)), get_tokens_result(tokens))
           : null;
};

const name_token = tokens => {
    const token = head(tokens);

    return get_token_type(token) === "symbol"
           ? make_parser_result(
               list("name", get_token(token)), get_tokens_result(tokens))
           : null;
};

const expression_statement = tokens => {
    const expr = expression(tokens);

    return !is_null(expr) && get_token_type(head(get_tokens_result(expr))) === ";"
           ? make_parser_result(get_ast_result(expr), tail(get_tokens_result(expr)))
           : null;
};

const function_application = (fun_expr, tokens) => {
    const left_paren = head(tokens);

    if(get_token_type(left_paren) !== "(") {
        return null;
    } else {
        const parsed_arg_exprs = delimit(expression, ",", true)(tail(tokens));

        if(is_null(parsed_arg_exprs)) {
            return null;
        } else {
            const right_paren = head(get_tokens_result(parsed_arg_exprs));

            return get_token_type(right_paren) !== ")"
                   ? error(
                       get_token_type(right_paren),
                       "Syntax error: missing right parenthesis")
                   : make_parser_result(
                       list("application",
                           fun_expr,
                           get_ast_result(parsed_arg_exprs)),
                       tail(get_tokens_result(parsed_arg_exprs)));
        }
    }
};

const conditional_expression = tokens => {
    const predicate = logical_composition(tokens);

    if(is_null(predicate)) {
        return null;
    } else {
        const question = head(get_tokens_result(predicate));

        if(get_token_type(question) !== "?") {
            return predicate;
        } else {
            const consequent_expression =
                conditional_expression(tail(get_tokens_result(predicate)));

            if(is_null(consequent_expression)) {
                error("Syntax error: conditional");
            } else {
                const colon = head(get_tokens_result(consequent_expression));

                if(get_token_type(colon) !== ":") {
                    error("Syntax error: conditional");
                } else {
                    const alternative_expression =
                        conditional_expression(tail(get_tokens_result(consequent_expression)));

                    return is_null(alternative_expression)
                           ? error("Syntax error: conditional")
                           : make_parser_result(list("conditional_expression",
                               get_ast_result(predicate),
                               get_ast_result(consequent_expression),
                               get_ast_result(alternative_expression)),
                               get_tokens_result(alternative_expression));
                }
            }
        }
    }
};

const conditional_statement = tokens => {
    if(get_token_type(head(tokens)) !== "if" ||
            get_token_type(head(tail(tokens))) !== "(") {
        return null;
    } else {
        const predicate = expression(tail(tail(tokens)));

        if(is_null(predicate) ||
                get_token_type(head(get_tokens_result(predicate))) !== ")") {
            error("Syntax error: conditional statement");
        } else {
            const consequent_block = block(tail(get_tokens_result(predicate)));

            if(is_null(consequent_block)) {
                error("Syntax error: conditional statement");
            } else if(get_token_type(head(get_tokens_result(consequent_block))) !== "else") {
                error("Syntax error: else required");
            } else {
                const alternative_if =
                    conditional_statement(tail(get_tokens_result(consequent_block)));

                if(!is_null(alternative_if)) {
                    return make_parser_result(list("conditional_statement",
                               get_ast_result(predicate),
                               get_ast_result(consequent_block),
                               get_ast_result(alternative_if)),
                               get_tokens_result(alternative_if));
                } else {
                    const alternative_block = block(tail(get_tokens_result(consequent_block)));

                    return is_null(alternative_block)
                           ? error("Syntax error: conditional statement")
                           : make_parser_result(list("conditional_statement",
                               get_ast_result(predicate),
                               get_ast_result(consequent_block),
                               get_ast_result(alternative_block)),
                               get_tokens_result(alternative_block));
                }
            }
        }
    }
};

const lambda_expression = tokens => {
    const inner = (names, tokens) => {
        if(get_token_type(head(tokens)) !== "=>") {
            return null;
        } else {
            const inner_expression = expression(tail(tokens));

            if(!is_null(inner_expression)) {
                return make_parser_result(
                    list("lambda_expression",
                        names,
                        list("block",
                            list("return_statement", get_ast_result(inner_expression)))),
                    get_tokens_result(inner_expression));
            } else {
                const inner_block = block(tail(tokens));

                return is_null(inner_block)
                       ? error("Syntax error: lambda")
                       : make_parser_result(
                           list("lambda_expression",
                               names,
                               get_ast_result(inner_block)),
                           get_tokens_result(inner_block));
            }
        }
    };

    if(get_token_type(head(tokens)) !== "(") {
        const name = name_token(tokens);

        return is_null(name)
               ? null
               : inner(list(get_ast_result(name)), get_tokens_result(name));
    } else {
        const names = delimit(name_token, ",", true)(tail(tokens));

        if(is_null(names) || get_token_type(head(get_tokens_result(names))) !== ")") {
            return null;
        } else {
            return inner(get_ast_result(names), tail(get_tokens_result(names)));
        }
    }
};

const sequence = tokens => {
    const seq = zero_or_more(statement)(tokens);

    return is_null(seq)
           ? null
           : make_parser_result(list(
               "sequence",
               get_ast_result(seq)),
               get_tokens_result(seq));
};

const block = tokens => {
    if(get_token_type(head(tokens)) !== "{") {
        return null;
    } else {
        const statements = sequence(tail(tokens));

        return is_null(statements) || get_token_type(head(get_tokens_result(statements))) !== "}"
               ? error("Syntax error: block")
               : make_parser_result(list(
                   "block",
                   get_ast_result(statements)),
                   tail(get_tokens_result(statements)));
    }
};

const return_statement = tokens => {
    if(get_token_type(head(tokens)) !== "return") {
        return null;
    } else {
        const expr = expression(tail(tokens));

        return !is_null(expr) && get_token_type(head(get_tokens_result(expr))) === ";"
               ? make_parser_result(list(
                   "return_statement",
                   get_ast_result(expr)),
                   tail(get_tokens_result(expr)))
               : error("Syntax error: return");
    }
};

const assignment = tokens => {
    const assign_name = name_token(tokens);

    if(is_null(assign_name) ||
            get_token_type(head(get_tokens_result(assign_name))) !== "=") {
        return conditional_expression(tokens);
    } else {
        const expr = conditional_expression(tail(get_tokens_result(assign_name)));

        return is_null(expr)
               ? error("Syntax error: assignment")
               : make_parser_result(list(
                   "assignment",
                   get_ast_result(assign_name),
                   get_ast_result(expr)), get_tokens_result(expr));
    }
};

const constant_variable_declaration = (head_token, type) => tokens => {
    if(get_token_type(head(tokens)) !== head_token) {
        return null;
    } else {
        const decl_name = name_token(tail(tokens));

        if(is_null(decl_name) ||
                get_token_type(head(get_tokens_result(decl_name))) !== "=") {
            error("Syntax error: constant declaration");
        } else {
            const expr = expression(tail(get_tokens_result(decl_name)));

            return is_null(expr) || get_token_type(head(get_tokens_result(expr))) !== ";"
                   ? error("Syntax error: declaration")
                   : make_parser_result(list(
                       type,
                       get_ast_result(decl_name),
                       get_ast_result(expr)),
                       tail(get_tokens_result(expr)));
        }
    }
};

const constant_declaration = constant_variable_declaration("const", "constant_declaration");
const variable_declaration = constant_variable_declaration("let", "variable_declaration");

const function_declaration = tokens => {
    if(get_token_type(head(tokens)) !== "function") {
        return null;
    } else {
        const func_name = name_token(tail(tokens));

        if(is_null(func_name) ||
                get_token_type(head(get_tokens_result(func_name))) !== "(") {
            error("Syntax error: function declaration");
        } else {
            const names = delimit(name_token, ",", true)(tail(get_tokens_result(func_name)));

            if(is_null(names) ||
                    get_token_type(head(get_tokens_result(names))) !== ")") {
                error("Syntax error: function declaration");
            } else {
                const func_block = block(tail(get_tokens_result(names)));

                return is_null(func_block)
                       ? error("Syntax error: function declaration")
                       : make_parser_result(list("function_declaration",
                           get_ast_result(func_name),
                           get_ast_result(names),
                           get_ast_result(func_block)), get_tokens_result(func_block));
            }
        }
    }
};

const statement = tokens => {
    return null_cor(list(
        expression_statement,
        conditional_statement,
        return_statement,
        constant_declaration,
        variable_declaration,
        function_declaration))(tokens);
};

const element_expression = tokens => {
    const lambda_expr = lambda_expression(tokens);

    if(!is_null(lambda_expr)) {
        return lambda_expr;
    } else if(get_token_type(head(tokens)) === "(") {
        const expr = expression(tail(tokens));

        return is_null(expr) || get_token_type(head(get_tokens_result(expr))) !== ")"
               ? error("Syntax error: element")
               : make_parser_result(get_ast_result(expr), tail(get_tokens_result(expr)));
    } else {
        return null_cor(list(literal_expression, name_token))(tokens);
    }
};

const parse_function_expression = tokens => {
    const expr = element_expression(tokens);
    const inner = (expr, tokens) => {
        const applied = function_application(expr, tokens);

        if(is_null(applied) ||
                get_token_type(head(get_tokens_result(applied))) !== "(") {
            return applied;
        } else {
            return inner(get_ast_result(applied), get_tokens_result(applied));
        }
    };

    return is_null(expr)
           ? null
           : get_token_type(head(get_tokens_result(expr))) === "("
           ? inner(get_ast_result(expr), get_tokens_result(expr))
           : expr;
};

const unary_operator = tokens => {
    if(get_token_type(head(tokens)) === "unary_operator") {
        const elm = parse_function_expression(tail(tokens));

        return is_null(elm)
               ? null
               : make_parser_result(list("unary_operator_combination",
                   get_token(head(tokens)),
                   get_ast_result(elm)), get_tokens_result(elm));
    } else if((get_token_type(head(tokens)) === "binary_operator" &&
            get_token(head(tokens)) === "-")) {
        const elm = parse_function_expression(tail(tokens));

        return is_null(elm)
               ? null
               : make_parser_result(list(
                   "unary_operator_combination",
                   "-unary",
                   get_ast_result(elm)),
                   get_tokens_result(elm));
    } else {
        return parse_function_expression(tokens);
    }
};

const expression_parser = (parser, op_list) => tokens => {
    const inner = (expr1, tokens) => {
        if(get_token_type(head(tokens)) !== "binary_operator" ||
                !memv(get_token(head(tokens)), op_list)) {
            return make_parser_result(expr1, tokens);
        } else {
            const expr2 = parser(tail(tokens));
            const expr_new = is_null(expr2)
                             ? error("Syntax error: expression")
                             : list("binary_operator_combination",
                                 get_token(head(tokens)),
                                 expr1,
                                 get_ast_result(expr2));

            return inner(expr_new, get_tokens_result(expr2));
        }
    };
    const expr1 = parser(tokens);

    return is_null(expr1)
           ? null
           : inner(get_ast_result(expr1), get_tokens_result(expr1));
};

const factor_expression = expression_parser(
    unary_operator,
    list("*", "/", "%"));
const term_expression = expression_parser(
    factor_expression,
    list("+", "-"));
const compare_expression = expression_parser(
    term_expression,
    list("<", ">", "<=", ">="));
const equal_expression = expression_parser(
    compare_expression,
    list("===", "!=="));

const logical_composition_base = (head_token, exp) => tokens => {
    const inner = (expr1, tokens) => {
        if(get_token_type(head(tokens)) !== "logical_composition" ||
                get_token(head(tokens)) !== head_token) {
            return make_parser_result(expr1, tokens);
        } else {
            const expr2 = exp(tail(tokens));
            const expr_new = is_null(expr2)
                             ? error("Syntax error: expression")
                             : list("logical_composition",
                                 get_token(head(tokens)),
                                 expr1,
                                 get_ast_result(expr2));

            return inner(expr_new, get_tokens_result(expr2));
        }
    };
    const expr1 = exp(tokens);

    return is_null(expr1)
           ? null
           : inner(get_ast_result(expr1), get_tokens_result(expr1));
};

const logical_composition_and = logical_composition_base("&&", equal_expression);
const logical_composition = logical_composition_base("||", logical_composition_and);

const expression = assignment;

const delimit = (parser, delimiter, allow_empty) => tokens => {
    if(is_null(tokens)) {
        return allow_empty
               ? make_parser_result(null, tokens)
               : error("Syntax error: list");
    } else {
        const parsed = parser(tokens);

        if(is_null(parsed)) {
            return allow_empty
                   ? make_parser_result(null, tokens)
                   : error("Syntax error: list");
        } else {
            const next_token = head(get_tokens_result(parsed));

            if(get_token_type(next_token) === delimiter) {
                const parsed_tail =
                    delimit(parser, delimiter, false)(tail(get_tokens_result(parsed)));

                return make_parser_result(
                    pair(get_ast_result(parsed),
                        get_ast_result(parsed_tail)),
                    get_tokens_result(parsed_tail));
            } else {
                return make_parser_result(
                    pair(get_ast_result(parsed), null),
                    get_tokens_result(parsed));
            }
        }
    }
};

const zero_or_one_more = (parser, allow_empty) => tokens => {
    if(is_null(tokens)) {
        return allow_empty
               ? make_parser_result(null, tokens)
               : error("Syntax error: list");
    } else {
        const parsed = parser(tokens);

        if(is_null(parsed)) {
            return allow_empty
                   ? make_parser_result(null, tokens)
                   : error("Syntax error: list");
        } else {
            const parsed_tail =
                zero_or_one_more(parser, true)(get_tokens_result(parsed));

            return make_parser_result(pair(get_ast_result(parsed),
                get_ast_result(parsed_tail)),
                get_tokens_result(parsed_tail));
        }
    }
};
const one_or_more = parser => zero_or_one_more(parser, false);
const zero_or_more = parser => zero_or_one_more(parser, true);

const null_cor = funcs => tokens => {
    if(is_null(funcs)) {
        return null;
    } else {
        const result = head(funcs)(tokens);

        return is_null(result) ? null_cor(tail(funcs))(tokens) : result;
    }
};

const parse_token = tokens => {
    const prg = sequence(tokens);

    return is_null(prg) || !is_null(get_tokens_result(prg))
           ? error("Syntax error: EOF")
           : get_ast_result(prg);
};

const parse = program => parse_token(tokenize(program, 0, null));


