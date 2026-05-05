/*
 * Unparser of SICP JS
 *
 * Copyright (c) 2026 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
const get_tag = element => head(element);

const literal_value = element => head(tail(element));
const get_name = element => head(tail(element));
const function_expression = element => head(tail(element));
const argument_expression_list = element => head(tail(tail(element)));
const get_predicate = element => head(tail(element));
const get_consequent_expression = element => head(tail(tail(element)));
const get_alternative_expression = element => head(tail(tail((tail(element)))));
const get_lambda_names = element => head(tail(element));
const get_lambda_statement = element => head(tail(tail(element)));
const get_assignment_name = element => head(tail(element));
const get_assignment_expression = element => head(tail(tail(element)));
const get_operator = element => head(tail(element));
const get_operand1 = element => head(tail(tail(element)));
const get_operand2 = element => head(tail(tail(tail(element))));

const expression_doc = tab => element => {
    const tag = get_tag(element);

    if(tag === "literal") {
        const literal = literal_value(element);

        return is_null(literal)
               ? make_text("null")
               : literal === true
               ? make_text("true")
               : literal === false
               ? make_text("false")
               : is_string(literal)
               ? make_concat(make_text('"'), make_concat(make_text(literal), make_text('"')))
               : is_number(literal)
               ? make_text(to_string(literal))
               : error("invalid literal", literal);
    } else if(tag === "name") {
        return make_text(get_name(element));
    } else if(tag === "application") {
        return make_concat(make_concat(expression_doc(tab)(function_expression(element)), make_text("(")),
            make_concat(make_nest(tab, make_concat(make_union(doc_nil, doc_linebreak),
                folddoc((x, y) => make_concat(x, make_concat(make_concat(make_text(","), make_union(make_text(" "), doc_line)), y)),
                    map(expression_doc(tab), argument_expression_list(element))))),
                make_text(")")));
    } else if(tag === "conditional_expression") {
        return make_concat(expression_doc(tab)(get_predicate(element)),
            make_nest(7, make_concat(make_concat(make_union(make_text(" "), doc_line), make_text("? ")),
                make_concat(expression_doc(tab)(get_consequent_expression(element)),
                    make_concat(make_concat(make_union(make_text(" "), doc_line), make_text(": ")),
                        expression_doc(tab)(get_alternative_expression(element)))))));
    } else if(tag === "lambda_expression") {
        return make_concat(
            make_concat(make_text("("), make_concat(
                make_nest(tab * 2, folddoc((x, y) => make_delimit_newline(x, y, ", "), map(expression_doc(tab), get_lambda_names(element)))), make_text(")"))),
            make_concat(make_text(" => "), statement_doc(tab)(get_lambda_statement(element))));
    } else if(tag === "assignment") {
        return make_concat(expression_doc(tab)(get_assignment_name(element)), make_concat(make_text(" ="),
            make_concat(make_union(make_text(" "), doc_line), make_nest(tab, expression_doc(tab)(get_assignment_expression(element))))));
    } else if(tag === "unary_operator_combination" || tag === "binary_operator_combination") {
        return operator_doc(tab)(element, -1);
    } else {
        error("invalid expression", tag);
    }
};

const operator_precedence = list(
    pair("*",  12), pair("/",  12), pair("%",  12),
    pair("+",  11), pair("-",  11),
    pair("<",   9), pair("<=",  9), pair(">",   9), pair(">=",  9),
    pair("===", 8), pair("!==", 8));

const operator_doc = tab => (element, precedence) => {
    const tag = get_tag(element);

    if(tag === "binary_operator_combination") {
        const op = get_operator(element);
        const thisprec = tail(assv(op, operator_precedence));
        const left = operator_doc(tab)(get_operand1(element), thisprec - 0.5);
        const right = operator_doc(tab)(get_operand2(element), thisprec + 0.5);
        const inner = make_concat(left, make_concat(
            make_nest(tab, make_concat(make_text(string_append(" ", op)), make_union(make_text(" "), doc_line))), right));

        return thisprec < precedence ? make_concat(make_text("("), make_concat(inner, make_text(")"))) : inner;
    } else if(tag === "unary_operator_combination") {
        const inner = operator_doc(tab)(get_operand1(element), 14);

        return make_concat(make_text(get_operator(element)), 14 < precedence ? bracket_break(tab)("(", inner, ")") : inner);
    } else {
        return expression_doc(tab)(element);
    }
};

const get_if_predicate = element => head(tail(element));
const get_if_consequent = element => head(tail(tail(element)));
const get_if_alternative = element => head(tail(tail(tail(element))));
const get_sequence = element => head(tail(element));
const get_block = element => head(tail(element));
const get_return = element => head(tail(element));
const get_declaration_name = element => head(tail(element));
const get_declaration_expression = element => head(tail(tail(element)));
const get_function_name = element => head(tail(element));
const get_function_args = element => head(tail(tail(element)));
const get_function_body = element => head(tail(tail(tail(element))));

const statement_doc = tab => element => {
    const tag = get_tag(element);

    if(tag === "conditional_statement") {
        return make_concat(make_text("if("), make_concat(expression_doc(tab)(get_if_predicate(element)), make_concat(make_text(")"),
            make_concat(statement_doc(tab)(get_if_consequent(element)),
                make_concat(make_text(" else "), statement_doc(tab)(get_if_alternative(element)))))));
    } else if(tag === "sequence") {
        return folddoc((x, y) => make_concat(x, make_concat(doc_line, y)), map(statement_doc(tab), get_sequence(element)));
    } else if(tag === "block") {
        return make_concat(make_text(" {"),
            make_concat(make_nest(tab, make_concat(doc_line, statement_doc(tab)(get_block(element)))),
                make_concat(doc_line, make_text("}"))));
    } else if(tag === "return_statement") {
        return make_concat(make_text("return "), make_concat(expression_doc(tab)(get_return(element)), make_text(";")));
    } else if(tag === "constant_declaration" || tag === "variable_declaration") {
        const label = tag === "constant_declaration" ? "const " : "let ";

        return make_concat(make_text(label), make_concat(expression_doc(tab)(get_declaration_name(element)),
            make_concat(make_text(" ="),
                make_nest(tab, make_concat(make_union(make_text(" "), doc_line),
                    make_concat(expression_doc(tab)(get_declaration_expression(element)), make_text(";")))))));
    } else if(tag === "function_declaration") {
        return make_concat(make_text("function "),
            make_concat(expression_doc(tab)(get_function_name(element)),
                make_concat(make_text("("),
                    make_concat(make_nest(tab * 2, folddoc((x, y) => make_concat(x, make_concat(make_concat(make_text(","), make_union(make_text(" "), doc_line)), y)),
                            map(expression_doc(tab), get_function_args(element)))),
                        make_concat(make_text(")"),
                            statement_doc(tab)(get_function_body(element)))))));
    } else {
        return make_concat(expression_doc(tab)(element), make_text(";"));
    }
};

const unparse = (w, tab, element) => pretty(w, $(statement_doc(tab)(element)));

