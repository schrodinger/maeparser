# maeparser
[![Build Status](https://travis-ci.org/schrodinger/maeparser.svg?branch=master)](https://travis-ci.org/schrodinger/maeparser)
[![Build_Status](https://ci.appveyor.com/api/projects/status/github/schrodinger/maeparser?branch=master&svg=true)](https://ci.appveyor.com/project/torcolvin/maeparser)

maeparser is a parser for [Schrodinger](https://www.schrodinger.com/) Maestro
files.

Structure files (.mae,.maegz,.mae.gz) can contain multiple structures
deliminated by "f_m_ct".  See [MaeConstants.hpp](MaeConstants.hpp) 
for standard block and property names.

To read a structure,

```C++
#include "Reader.hpp"

...

FILE* f = fopen("test.mae", "r");
schrodinger::mae::Reader r(f);

std::shared_ptr<schrodinger::mae::Block> b;
while ((b = r.next(schrodinger::mae::CT_BLOCK)) != nullptr) {
  // Parse structure
}
fclose(f);
```

See also test/UsageDemo.cpp, which reads an example structure and
stores it in a dummy Molecule class.

Background
==========

Why do we have a recursive descent parser for mae files instead of using a
parser generator like ANTLR or lex/yacc? The main reasons are that the mae
format language is 1) pretty simple, 2) unlikely to change significantly,
and 3) not a context free grammar. In addition, speed of parsing these
files is important.

In what way is the current version of the language not a CFG? Special tokens
like block opener `{` and key/value separator `:::` can also be string
values because the quotes on string values are not required. This results in
complication and pain in attempts to define a grammar.


