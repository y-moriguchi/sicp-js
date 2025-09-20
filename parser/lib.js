/*
 * Parser of SICP JS
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
const $ = v => (console.log(v), v);
const error = (sym, msg) => { throw new Error(sym + ":" + msg); };
const undef = void 0;

const is_null = val => val === null;
const head = pair => pair.at(0);
const tail = pair => pair.at(1);
const pair = (head, rest) => [head, rest];
const list = (...args) => {
    return args.length > 0
           ? pair(args.at(0), list(...args.slice(1)))
           : null;
};
const list_to_array = list => {
    return list === null
           ? []
           : [head(list)].concat(list_to_array(tail(list)));
};
const reverse = list => {
    const inner = (list, reversed) => {
        return list === null
               ? reversed
               : inner(tail(list), pair(head(list), reversed));
    };

    return inner(list, null);
};
const memv = (obj, list) => is_null(list) ? false : obj === head(list) ? list : memv(obj, tail(list));
const assv = (obj, list) => {
    return is_null(list)
           ? false
           : obj === head(head(list))
           ? head(list)
           : assv(obj, tail(list));
};
const map = (f, list) => {
    return is_null(list)
           ? null
           : pair(f(head(list)), map(f, tail(list)));
};
const list_ref = (list, i) => {
    return i > 0
           ? list_ref(tail(list), i - 1)
           : head(list);
};
const is_pair = obj => Array.isArray(obj);
const append = (list1, list2) => {
    return is_null(list1)
           ? list2
           : pair(head(list1), append(tail(list1), list2));
};
const length = list => {
    return is_null(list)
           ? 0
           : length(tail(list)) + 1;
};
const accumulate = (f, init, list) => {
    return is_null(list)
           ? init
           : f(head(list), accumulate(f, init, tail(list)));
};
const set_head = (list, val) => list[0] = val;
const set_tail = (list, val) => list[1] = val;

const apply_in_underlying_javascript = (f, args) => f(...list_to_array(args));
const display = msg => console.log(msg);

const math_abs = v => Math.abs(v);
const math_PI = Math.PI;
const math_E = Math.E;

const is_boolean = obj => typeof obj === "boolean";

const string_ref = (s, i) => s.at(i);
const string_length = s => s.length;
const string_append = (...args) => args.length > 0 ? args.at(0).toString() + string_append(...args.slice(1)) : "";
const substring = (s, begin, end) => s.substring(begin, end);
const char_to_integer = ch => ch.charCodeAt(0);
const is_whitespace = ch => /[ \t\n]/.test(ch);
const is_alphabetic = ch => /[a-zA-Z]/.test(ch);
const is_numeric = ch => /[0-9]/.test(ch);

const pow_int = (a, p) => Math.pow(a, p);





