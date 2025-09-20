const fs = require('fs');

const lib = fs.readFileSync('lib.js', 'utf-8');
const parser = fs.readFileSync('parser.js', 'utf-8');
const mod = lib + '\n' + parser + '\nreturn parse;';

const parse = new Function(mod)();

const list_expr = parse(parser);
const list_string0 = JSON.stringify(list_expr, null, 1);
const list_string = list_string0.replaceAll(/^ +/gm, '').replaceAll(/"\\\\\\""/gm, "'\"'").replaceAll(/\\\\\\\\/gm, '\\');

fs.writeFileSync('jsparser.txt', list_string);

