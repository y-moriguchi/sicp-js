/*
 * Regular Expression Engine Written By SICP JS
 *
 * Copyright (c) 2025 Yuichiro MORIGUCHI
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 **/
const make_len = i => {
    return i === 0 ? null : pair(null, make_len(i - 1));
};

const ref_and_set_head = (lst, i, val) => {
    return is_null(lst)
           ? error("Internal error")
           : i === 0
           ? set_head(lst, val)
           : ref_and_set_head(tail(lst), i - 1, val);
};

const match_char_range = (ch1, ch2) => {
    const chi1 = char_to_integer(ch1);
    const chi2 = char_to_integer(ch2);

    return (s, i, cap, k) => {
        if(i < string_length(s)) {
            const ch = char_to_integer(string_ref(s, i));

            return ch >= chi1 && ch <= chi2 ? k(s, i + 1, cap) : null;
        } else {
            return null;
        }
    };
};
const match_char = ch => match_char_range(ch, ch);

const match_begin = (s, i, cap, k) => i === 0 ? k(s, i, cap) : null;
const match_end = (s, i, cap, k) => i === string_length(s) ? k(s, i, cap) : null;
const match_any = (s, i, cap, k) => i < string_length(s) ? k(s, i + 1, cap) : null;

const match_negate_char = ptn => (s, i, cap, k) => {
    const r = ptn(s, i, cap, k);

    return r === null ? k(s, i + 1, cap) : null;
};

const match_sequence = seq => {
    return is_null(seq)
           ? (s, i, cap, k) => k(s, i, cap)
           : (s, i, cap, k) => {
               const nextk = (s, i, cap) => match_sequence(tail(seq))(s, i, cap, k);

               return head(seq)(s, i, cap, nextk);
           };
};

const match_one_or_more = ptn => (s, i, cap, k) => {
    const inner = (s, i, cap) => {
        return ptn(s, i, cap, (s, i, cap) => {
            const r = inner(s, i, cap);

            return is_null(r) ? k(s, i, cap) : r;
        });
    };

    return inner(s, i, cap);
};

const match_zero_or_more_not_greedy = ptn => (s, i, cap, k) => {
    const inner = (s, i, cap) => {
        const r = k(s, i, cap);

        return is_null(r) ? ptn(s, i, cap, inner) : r;
    };

    return inner(s, i, cap);
};

const match_option = ptn => (s, i, cap, k) => {
    const r = ptn(s, i, cap, k);

    return r === null ? k(s, i, cap) : r;
};

const match_option_not_greedy = ptn => (s, i, cap, k) => {
    const r = k(s, i, cap);

    return r === null ? ptn(s, i, cap, k) : r;
};

const match_alternate = alts => (s, i, cap, k) => {
    if(is_null(alts)) {
        return null;
    } else {
        const r = head(alts)(s, i, cap, k);

        return is_null(r) ? match_alternate(tail(alts))(s, i, cap, k) : r;
    }
};

const match_capture = (ptn, no) => (s, i, cap, k) => {
    const begin = i;
    return ptn(s, i, cap, (s, i, cap) => {
        ref_and_set_head(cap, no, substring(s, begin, i));
        return k(s, i, cap);
    });
};

const match_refer = no => (s, i, cap, k) => {
    const begin = i;
    const captured = list_ref(cap, no);
    const inner = (s, i, cap) => {
        if(i > string_length(s)) {
            return null;
        } else if(i - begin === string_length(captured)) {
            return k(s, i, cap);
        } else if(string_ref(s, i) === string_ref(captured, i - begin)) {
            return inner(s, i + 1, cap);
        } else {
            return null;
        }
    };

    return inner(s, i, cap);
};

const match_lookahead = ptn => (s, i, cap, k) => {
    const r = ptn(s, i, cap, (s, i, cap) => i);

    return is_null(r) ? null : k(s, i, cap);
};

const match_negative_lookahead = ptn => (s, i, cap, k) => {
    const r = ptn(s, i, cap, (s, i, cap) => i);

    return is_null(r) ? k(s, i, cap) : null;
};

const match_atomic = ptn => (s, i, cap, k) => {
    const r = ptn(s, i, cap, (s, i, cap) => i);

    return is_null(r) ? null : k(s, r, cap);
};

const integer_A = char_to_integer("A");
const integer_F = char_to_integer("F");
const integer_a = char_to_integer("a");
const integer_f = char_to_integer("f");
const integer_0 = char_to_integer("0");
const integer_9 = char_to_integer("9");
const hex_to_digit = ch => {
    const chi = char_to_integer(ch);

    if(chi >= integer_A && chi <= integer_F) {
        return chi - integer_A + 10;
    } else if(chi >= integer_a && chi <= integer_f) {
        return chi - integer_a + 10;
    } else if(chi >= integer_0 && chi <= integer_9) {
        return chi - integer_0;
    } else {
        error("Regex syntax error");
    }
};

const parse_regex_escape = (slist, capture_no, k) => {
    if(is_null(slist)) {
        error("Regex syntax error");
    } else if(head(slist) === "n") {
        return k(tail(slist), match_char("\n"), capture_no);
    } else if(head(slist) === "r") {
        return k(tail(slist), match_char("\r"), capture_no);
    } else if(head(slist) === "t") {
        return k(tail(slist), match_char("\t"), capture_no);
    } else if(head(slist) === "x") {
        if(is_null(tail(slist))) {
            error("Regex syntax error");
        } else {
            const x1 = hex_to_digit(head(tail(slist)));
            const x2 = hex_to_digit(head(tail(tail(slist))));

            return k(tail(tail(tail(slist))),
                     match_char(integer_to_char(x1 * 16 + x2)),
                     capture_no);
        }
    } else {
        return k(tail(slist), match_char(head(slist)), capture_no);
    }
};

const parse_regex_charset_element = (slist, capture_no, k) => {
    if(is_null(slist)) {
        error("Regex syntax error");
    } else if(!is_null(tail(slist)) &&
            !is_null(tail(tail(slist))) &&
            head(tail(slist)) === "-") {
        return k(
            tail(tail(tail(slist))),
            match_char_range(head(slist), head(tail(tail(slist)))),
            capture_no);
    } else if(head(slist) === "\\") {
        return parse_regex_escape(tail(slist), capture_no, k);
    } else {
        return k(tail(slist), match_char(head(slist)), capture_no);
    }
};

const parse_regex_charset_inner = (slist, capture_no, k) => {
    const inner = (slist, matcher, capture_no) => {
        if(is_null(slist)) {
            error("Regex syntax error");
        } else if(head(slist) === "]") {
            return k(tail(slist), matcher, capture_no);
        } else {
            return parse_regex_charset_element(
                slist,
                capture_no,
                (slist, matcher2, capture_no) => {
                    const matcher_new = is_null(matcher)
                                        ? matcher2
                                        : match_alternate(
                                            list(matcher, matcher2));

                    return inner(slist, matcher_new, capture_no);
                });
        }
    };

    return inner(slist, null, capture_no);
};

const parse_regex_charset = (slist, capture_no, k) => {
    if(is_null(slist)) {
        error("Regex syntax error");
    } else if(head(slist) === "^") {
        return parse_regex_charset_inner(
            tail(slist),
            capture_no,
            (slist, matcher, capture_no) => {
                return k(slist, match_negate_char(matcher), capture_no);
            });
    } else {
        return parse_regex_charset_inner(slist, capture_no, k);
    }
};

const is_question = (ch, slist) => {
    return is_pair(tail(slist)) &&
           is_pair(tail(tail(slist))) &&
           head(tail(slist)) === "?" &&
           head(tail(tail(slist))) === ch;
};

const parse_regex_element = (slist, capture_no, k) => {
    const check_right_paren = slist => {
        if(!(is_pair(slist) && head(slist) === ")")) {
            error("Regex syntax error");
        } else { }
    };
    const return_question = f => (slist, capture_no, k) => {
        return parse_regex_inner(
            tail(tail(tail(slist))),
            capture_no,
            (slist, matcher, capture_no) => {
                check_right_paren(slist);
                return k(tail(slist), f(matcher), capture_no);
            });
    };

    if(is_null(slist)) {
        error("Regex syntax error");
    } else if(head(slist) === "(") {
        if(is_question(":", slist)) {
            return return_question(v => v)(slist, capture_no, k);
        } else if(is_question("=", slist)) {
            return return_question(v => match_lookahead(v))(
                slist, capture_no, k);
        } else if(is_question("!", slist)) {
            return return_question(v => match_negative_lookahead(v))(
                slist, capture_no, k);
        } else if(is_question(">", slist)) {
            return return_question(v => match_atomic(v))(
                slist, capture_no, k);
        } else {
            return parse_regex_inner(
                tail(slist),
                capture_no,
                (slist, matcher, capture_no) => {
                    check_right_paren(slist);
                    return k(tail(slist), match_capture(matcher, capture_no), capture_no + 1);
                });
        }
    } else if(head(slist) === "\\") {
        if(is_null(tail(slist))) {
            error("Regex syntax error");
        } else if(is_numeric(head(tail(slist)))) {
            return k(tail(tail(slist)),
                     match_refer(
                         char_to_integer(head(tail(slist))) - char_to_integer("1")),
                     capture_no);
        } else {
            return parse_regex_escape(
                tail(slist),
                capture_no,
                (slist, matcher, capture_no) => k(slist, matcher, capture_no));
        }
    } else if(head(slist) === "[") {
        return parse_regex_charset(tail(slist), capture_no, k);
    } else if(head(slist) === "^") {
        return k(tail(slist), match_begin, capture_no);
    } else if(head(slist) === "$") {
        return k(tail(slist), match_end, capture_no);
    } else if(head(slist) === ".") {
        return k(tail(slist), match_any, capture_no);
    } else {
        return k(tail(slist), match_char(head(slist)), capture_no);
    }
};

const parse_regex_closure = (slist, capture_no, k) => {
    return parse_regex_element(slist, capture_no, (slist, matcher, capture_no) => {
        if(is_null(slist)) {
            return k(slist, matcher, capture_no);
        } else if(head(slist) === "+") {
            if(is_pair(tail(slist)) && head(tail(slist)) === "?") {
                return k(tail(tail(slist)),
                         match_sequence(list(
                             match_zero_or_more_not_greedy(matcher), matcher)),
                         capture_no);
            } else if(is_pair(tail(slist)) && head(tail(slist)) === "+") {
                return k(tail(slist),
                         match_atomic(match_one_or_more(matcher)),
                         capture_no);
            } else {
                return k(tail(slist), match_one_or_more(matcher), capture_no);
            }
        } else if(head(slist) === "*") {
            if(is_pair(tail(slist)) && head(tail(slist)) === "?") {
                return k(tail(tail(slist)),
                         match_zero_or_more_not_greedy(matcher),
                         capture_no);
            } else if(is_pair(tail(slist)) && head(tail(slist)) === "+") {
                return k(tail(tail(slist)),
                         match_atomic(
                             match_option(match_one_or_more(matcher))),
                         capture_no);
            } else {
                return k(tail(slist), match_option(match_one_or_more(matcher)), capture_no);
            }
        } else if(head(slist) === "?") {
            if(is_pair(tail(slist)) && head(tail(slist)) === "?") {
                return k(tail(tail(slist)),
                         match_option_not_greedy(matcher),
                         capture_no);
            } else if(is_pair(tail(slist)) && head(tail(slist)) === "+") {
                return k(tail(slist),
                         match_atomic(match_option(matcher)),
                         capture_no);
            } else {
                return k(tail(slist), match_option(matcher), capture_no);
            }
        } else {
            return k(slist, matcher, capture_no);
        }
    });
};

const parse_regex_sequence = (slist, capture_no, k) => {
    const inner = (slist, matcher, capture_no) => {
        if(is_null(slist) || head(slist) === "|" || head(slist) === ")") {
            return k(slist, matcher, capture_no);
        } else {
            return parse_regex_sequence(
                slist,
                capture_no,
                (slist, matcher2, capture_no) => {
                    return inner(
                        slist,
                        match_sequence(list(matcher, matcher2)),
                        capture_no);
                });
        }
    };

    return parse_regex_closure(slist, capture_no, inner);
};

const parse_regex_alternate = (slist, capture_no, k) => {
    const inner = (slist, matcher, capture_no) => {
        if(is_null(slist) || head(slist) === ")") {
            return k(slist, matcher, capture_no);
        } else if(head(slist) === "|") {
            return parse_regex_alternate(
                tail(slist),
                capture_no,
                (slist, matcher2, capture_no) => {
                    return inner(
                        slist,
                        match_alternate(list(matcher, matcher2)),
                        capture_no);
                });
        } else {
            return parse_regex_sequence(slist, capture_no, inner);
        }
    };

    return parse_regex_sequence(slist, capture_no, inner);
};

const parse_regex_inner = parse_regex_alternate;
const parse_regex = s => {
    const last = (slist, matcher, capture_no) => {
        if(is_null(slist)) {
            return matcher;
        } else {
            error("Regex syntax error");
        }
    };

    return parse_regex_inner(string_to_list(s), 0, last);
};

const match = ptn => s => ptn(s, 0, make_len(10), (s, i) => i);

