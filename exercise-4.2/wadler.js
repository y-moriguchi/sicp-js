/*
 * Unparser of SICP JS
 *
 * Copyright (c) 2026 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
const doc_nil = list("NIL");

const make_concat = (a, b, c) => c !== undefined ? error(c.toString()) : list("<>", a, b);
const get_concat_left = c => head(tail(c));
const get_concat_right = c => head(tail(tail(c)));

const make_nest = (i, d) => list("NEST", i, d);
const get_nest_pos = n => head(tail(n));
const get_nest_doc = n => head(tail(tail(n)));

const make_text = t => list("TEXT", t);
const get_text = t => head(tail(t));

const doc_line = list("LINE");
const doc_linebreak = list("LINEBREAK");

const make_union = (a, b) => list("<|>", a, b);
const get_union_left = c => head(tail(c));
const get_union_right = c => head(tail(tail(c)));

const make_pospair = (i, d) => pair(i, d);
const get_position = p => head(p);
const get_doc = p => tail(p);

const get_instruction = c => head(c);

const string_repeat = (i, s, org) => i > 0 ? string_repeat(i - 1, s, string_append(org, s)) : org;

const doc_int_nil = list("Nil");

const make_int_text = (s, rest) => list("Text", s, rest);
const get_int_text = text => head(tail(text));
const get_int_text_rest = text => head(tail(tail(text)));

const make_int_line = (i, rest) => list("Line", i, rest);
const get_int_line_pos = line => head(tail(line));
const get_int_line_rest = line => head(tail(tail(line)));

const make_int_linebreak = (i, rest) => list("LineBreak", i, rest);
const get_int_linebreak_pos = line => head(tail(line));
const get_int_linebreak_rest = line => head(tail(tail(line)));

const best = (w, k, x) => be(w, k, list(make_pospair(0, x)));
const be = (w, k, stack) => {
    if(is_null(stack)) {
        return doc_int_nil;
    } else {
        const stack_head = head(stack);
        const position = get_position(stack_head);
        const doc = get_doc(stack_head);
        const instruction = get_instruction(doc);
        const z = tail(stack);

        if(instruction === "NIL") {
            return be(w, k, z);
        } else if(instruction === "<>") {
            return be(w, k,
                pair(make_pospair(position, get_concat_left(doc)),
                pair(make_pospair(position, get_concat_right(doc)), z)));
        } else if(instruction === "NEST") {
            return be(w, k, pair(make_pospair(position + get_nest_pos(doc), get_nest_doc(doc)), z));
        } else if(instruction === "TEXT") {
            return make_int_text(get_text(doc), be(w, k + string_length(get_text(doc)), z));
        } else if(instruction === "LINE") {
            return make_int_line(position, be(w, position, z));
        } else if(instruction === "LINEBREAK") {
            return make_int_linebreak(position, be(w, position, z));
        } else if(instruction === "<|>") {
            const x = be(w, k, pair(make_pospair(position, get_union_left(doc)), z));
            const y = be(w, k, pair(make_pospair(position, get_union_right(doc)), z));

            return fits(w - k, x) ? x : y;
        } else {
            error(string_append("invalid instruction", instruction));
        }
    }
};

const fits = (w, x) => {
    const instruction = get_instruction(x);

    return w < 0
           ? false
           : instruction === "Nil"
           ? true
           : instruction === "Text"
           ? fits(w - string_length(get_int_text(x)), get_int_text_rest(x))
           : instruction === "Line" || instruction === "LineBreak"
           ? true
           : error(string_append("invalid instruction", instruction));
};

const group = x => make_union(flatten(x), x);
const flatten = doc => {
    const instruction = get_instruction(doc);

    return instruction === "NIL"
           ? doc
           : instruction === "<>"
           ? make_concat(flatten(get_concat_left(doc)), flatten(get_concat_right(doc)))
           : instruction === "NEST"
           ? make_nest(get_nest_pos(doc), flatten(get_nest_doc(doc)))
           : instruction === "TEXT"
           ? doc
           : instruction === "LINE"
           ? make_text(" ")
           : instruction === "LINEBREAK"
           ? doc_nil
           : instruction === "<|>"
           ? flatten(get_union_left(doc))
           : error("invalid instruction", instruction);
};
 
const layout = doc => {
    const instruction = get_instruction(doc);

    return instruction === "Nil"
           ? ""
           : instruction === "Text"
           ? string_append(get_int_text(doc), layout(get_int_text_rest(doc)))
           : instruction === "Line"
           ? string_append(string_repeat(get_int_line_pos(doc), " ", "\n"), layout(get_int_line_rest(doc)))
           : instruction === "LineBreak"
           ? string_append(string_repeat(get_int_linebreak_pos(doc), " ", "\n"), layout(get_int_linebreak_rest(doc)))
           : error("invalid instruction", instruction);
};

const pretty = (w, x) => layout(best(w, 0, x));

const make_delimit = (x, y, delimiter) => make_concat(x, make_concat(make_text(delimiter), y));
const make_newline = (x, y) => make_concat(x, make_concat(doc_line, y));
const folddoc = (f, lst) => is_null(lst) ? doc_nil : is_null(tail(lst)) ? head(lst) : f(head(lst), folddoc(f, tail(lst)));
const spread = (x, delimiter) => folddoc((x, y) => make_delimit(x, y, delimiter), x);
const stack = x => folddoc(make_newline, x);
const bracket = tab => (l, x, r) => group(make_concat(make_text(l), make_concat(
    make_nest(tab, make_concat(doc_line, x)), make_concat(doc_line, make_text(r)))));
const bracket_break = tab => (l, x, r) => group(make_concat(make_text(l), make_concat(
    make_nest(tab, make_concat(doc_linebreak, x)), make_concat(doc_linebreak, make_text(r)))));
const make_delimit_newline = (x, y, s) => make_concat(x, make_concat(make_union(make_text(s), doc_line), y));
const fill = lst => {
    const x = head(lst);

    if(is_null(tail(lst))) {
        return x;
    } else {
        const y = head(tail(lst));
        const zs = tail(tail(lst));

        return make_union(make_delimit(flatten(x), fill(pair(flatten(y), zs)), " "), make_newline(x, fill(pair(y, zs))));
    }
};

