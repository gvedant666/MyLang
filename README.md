# MyLang — Tiny educational compiler

A compact single-file compiler front-to-back for a tiny toy language ("MyLang"). It tokenizes source code, parses it into an AST, and emits a small assembly-like text format.

---

## Table of Contents

* [Overview](#overview)
* [Language & Grammar](#language--grammar)
* [Compiler Pipeline (Architecture)](#compiler-pipeline-architecture)
* [Parser / AST (diagram)](#parser--ast-diagram)
* [Example: source -> tokens -> AST -> generated assembly](#example)
* [Build & Run](#build--run)
* [Project structure / files](#project-structure--files)
* [Design & Implementation notes](#design--implementation-notes)
* [Limitations](#limitations)
* [Future work](#future-work)
* [License](#license)

---

## Overview

This project implements a tiny compiler for a toy language named **MyLang**. The compiler is small and educational — intended to show the basic stages of compilation: **tokenization**, **parsing** (producing an AST), and **code generation** (to a minimal assembly-like representation).

Key goals:

* Demonstrate a clean, hand-written tokenizer and recursive-descent parser.
* Build a compact AST using `unique_ptr` and `std::variant`.
* Emit a readable, minimal assembly-like output.

The implementation (headers + `Main.cpp`) is intentionally minimal and easy to extend for students learning compilers.

---

## Language & Grammar (informal)

MyLang currently supports two statement forms:

1. **Integer declaration with initializer**

```
int <identifier> = <expression> ;
```

2. **Exit call**

```
exit ( <expression> ) ;
```

Expressions are currently very small and composed of a single term. A term is either an integer literal or an identifier.

A compact BNF-ish view (informal):

```
<program>   ::= { <stmt> }
<stmt>      ::= "int" <ident> "=" <expr> ";"
              |  "exit" "(" <expr> ")" ";"
<expr>      ::= <term>
<term>      ::= <int_literal> | <ident>
```

**Tokens recognized** (the tokenizer):

* `int` (keyword)
* `exit` (keyword)
* `ident` (identifier)
* `INT` (integer literal)
* `(`, `)` , `;`, `=`

---

## Compiler pipeline (architecture)

High level pipeline (source → final output):

```
Source file (.sl)
   ↓
Tokenizer (lexical analysis)  — produces a vector<Token>
   ↓
Parser (recursive descent)    — produces an AST (NodeProg)
   ↓
Generator (codegen)          — walks the AST and emits assembly-like text
   ↓
Assembly-like output (text file printed to disk)
```

Mermaid sequence (optional, GitHub might not render without enabling mermaid):

```mermaid
flowchart LR
  A[Source (text.sl)] --> B(Tokenizer)
  B --> C(Parser)
  C --> D(Generator)
  D --> E[Assembly output (.asm-like text)]
```

---

## Parser / AST (diagram)

The AST is made of these node types (extracted from `parser.hpp`):

* `NodeProg` — vector of statements
* `NodeStmt` — variant: can be `NodeStmtInt` (int declaration) or `NodeStmtExit` (exit statement)
* `NodeStmtInt` — holds an identifier (`NodeIdent`) and an expression (`NodeExpr`)
* `NodeStmtExit` — holds an expression (`NodeExpr`)
* `NodeExpr` — holds a `NodeTerm`
* `NodeTerm` — variant: `NodeIntLit` or `NodeIdent`
* `NodeIntLit` — integer literal token
* `NodeIdent` — identifier token

### Example parse tree (ASCII)

Given source:

```text
int x = 5;
exit(x);
```

The parser produces an AST like:

```
NodeProg
├─ NodeStmt (stmt_type = Ident)
│  └─ NodeStmtInt
│     ├─ NodeIdent (ident: "x")
│     └─ NodeExpr
│        └─ NodeTerm
│           └─ NodeIntLit (value: 5)
└─ NodeStmt (stmt_type = Exit)
   └─ NodeStmtExit
      └─ NodeExpr
         └─ NodeTerm
            └─ NodeIdent (ident: "x")
```

This diagram matches the `struct` definitions in `parser.hpp` (e.g. `NodeStmt` contains a `variant` of possible statement node types, `NodeProg` stores a `vector` of `unique_ptr<NodeStmt>`).

---

## Example — full flow

**Input file** (`text.sl`):

```c
int a = 42;
int b = 3;
exit(a);
```

**Tokenization** (conceptual):

```
[int_, ident(a), assign, INT(42), semicolon, int_, ident(b), assign, INT(3), semicolon, return_, open_paren, ident(a), close_paren, semicolon]
```

**AST** (conceptual — similar to the ASCII tree above)

**Generated assembly-like output** (the generator writes a simple text format). Example output for the `exit(a);` case would contain instructions like:

```asm
.text

start:
    ldi A 42
    sta %a
    ldi A 3
    sta %b
    ldi A 42    ; depending on generator implementation (may load expr value)
    lda %r
    out 0
    hlt
```

> Note: the exact generated text is dependent on `asm_generator.hpp`'s current code generation logic. It emits opcodes like `ldi`, `sta %<ident>`, `lda %r`, `out 0`, and `hlt`.

---

## Build & Run

`Main.cpp` is the only compilation unit; the rest of the compiler is implemented in header files. The project uses standard C++ (`<memory>`, `std::variant`, etc.).

### Build (Linux / macOS / Windows with mingw)

Using `g++` (C++17 or later):

```bash
# from project root (where Main.cpp lives)
# compile
g++ -std=c++17 -O2 Main.cpp -o mylang

# run (provide a path to your .sl file)
./mylang path/to/text.sl
```

On Windows (MinGW/MSYS) you can use the provided hint in `Main.cpp`:

```bash
mingw32-make clean && mingw32-make run
```

> The program expects (per `Main.cpp`) a path like `C:/Code/C++/Compiler/SimpleLang/text.sl` in the variable `filePath`. Edit `Main.cpp` or pass the desired path as a CLI argument if you add argument parsing.

### Expected behaviour

* The program tokenizes the input file, parses it to an AST and then calls the `Generator` to produce the assembly-like output. Output and debug prints are shown on stdout and the generator writes to a file (see `asm_generator.hpp` for output file details).

---

## Project structure / files

```
/ (project root)
├─ Main.cpp            # driver: reads a .sl file, tokenizes, parses, generates
├─ tokenizer.hpp       # lexer: Token, Tokenizer, TokenType enum
├─ parser.hpp          # recursive-descent parser + AST node structs
├─ asm_generator.hpp   # simple backend that walks the AST and emits text assembly
└─ text.sl             # example input file (not provided here)
```

Brief responsibilities:

* **tokenizer.hpp** — implements `Tokenizer` which reads characters, groups them into `Token` objects and recognizes keywords `int` and `exit` (mapped to `int_` and `return_` tokens respectively).
* **parser.hpp** — implements a small recursive-descent parser that builds the AST nodes (`NodeProg`, `NodeStmt`, `NodeExpr`, etc.). It validates token order and reports parse failures with `std::cerr`.
* **asm\_generator.hpp** — traverses the AST and emits target text instructions (e.g. `ldi`, `sta`, `lda`, `out`, `hlt`).

---

## Design & Implementation notes

* AST nodes use `std::unique_ptr` for ownership and `std::variant` where a node can be one of several options. This makes node types explicit and simple to match/visit.
* The parser is a straightforward recursive-descent parser, with helper functions for `parse_term()`, `parse_expr()`, and `parse_stmt()`.
* The code generator (`Generator`) walks the AST; there are small helper `generate_*` functions (`generate_term`, `generate_expr`, `generate_stmt_int`, `generate_exit`, etc.) that write textual instructions to an output stream.
* Error handling is done with `std::cerr` and `exit(EXIT_FAILURE)` on unrecoverable errors (suitable for an educational project but not for production compilers).

---

## Limitations

* The language is tiny: only `int` declarations and `exit(...)` exist.
* Expressions are single-term only (no binary operators, no precedence handling).
* No scopes, no symbol table beyond naive codegen that uses identifier names directly.
* Error reporting is minimal.
* No command-line argument parsing currently — `Main.cpp` uses a hardcoded path.

---

## Future work

* Add binary expressions (`+`, `-`, `*`, `/`) and full expression parsing (shunting-yard or recursive-descent with precedence).
* Implement a symbol table and typed variable declarations / storage layout.
* Expand code generation to target a known assembly (x86-64/NASM/AT\&T) or emit WebAssembly or LLVM IR.
* Improve error messages with line/column info in the tokenizer and parser.
* Add CLI support: `mylang <input.sl> -o out.s`.
* Add a test suite and sample `.sl` programs, plus a small Makefile / CMake configuration.

---

## Appendix — useful snippets

**Sample `text.sl`:**

```c
int x = 5;
int y = 10;
exit(x);
```

**How to quickly test** (after compilation):

```bash
# edit Main.cpp to point to your text.sl or modify Main.cpp to accept argv[1]
./mylang
# check the generated .asm-like output file (see asm_generator.hpp for filename)
```

---

## License

This README does not change the project license. If you want to add one, consider using the MIT license for an educational project.

---

If you want, I can also:

* Produce a nicely-formatted `Makefile` or `CMakeLists.txt`.
* Add example `.sl` test files and a small test runner.
* Convert the ASCII diagrams into SVG/PNG that you can include in the repo README.

Tell me which of the above you'd like next and I will add it directly to the repo README file.
