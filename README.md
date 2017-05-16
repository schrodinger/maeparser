# maeparser

maeparser is parser for [Schrodinger](https://www.schrodinger.com/) Maestro
files.

Structure files (.mae,.maegz,.mae.gz) can contain multiple structures
deliminated by "f_m_ct".

To read a structure,

```C++
FILE* f = fopen("test.mae", "r");
Reader r(f);

std::shared_ptr<Block> b;
while ((b = r.next("f_m_ct")) != nullptr) {
  // Parse structure
}
fclose(f);
```

Background
==========

Why do we have a recursive descent parser for mae files instead of using a
parser generator like ANTLR or lex/yacc? The main reasons are that the mae
format language 1) is pretty simple, 2) unlikely to change significantly,
3) is not a context free grammar, and 4) speed of parsing these
files is important.

In what way is the current version of the language not a CFG? Special tokens
like block opener '{' and key/value separator ':::' can also be string
values because the quotes on string values are not required. This results in
complication and pain in attempts to define a grammar.


