
# seng — Simple English Programming Language

[![Build Status](https://github.com/KANAGARAJ-M/SENG/actions/workflows/build.yml/badge.svg)](https://github.com/KANAGARAJ-M/SENG/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/KANAGARAJ-M?color=ea4aaa)](https://github.com/sponsors/KANAGARAJ-M)

> _Write code the way you think._

**seng** (Simple English) is a programming language designed so that even non-programmers can read, write, and understand code. Its syntax is plain English.

---

## 🌎 Ideology, Mission & Vision

### 🧩 Ideology: Logic over Syntax
We believe that programming is a human right, not a technical privilege. The ability to instruct a computer should not depend on memorizing abstract symbols. `seng` focuses on the **logic** of your thoughts rather than the **syntax** of the machine.

### 🎯 Mission: Democratizing Code
Our mission is to provide a zero-friction entry point into software development. We aim to provide educators and self-learners with a tool that teaches "computational thinking" without the frustration of traditional coding barriers.

### 🔭 Vision: Natural Language as Code
We envision a world where anyone can create software as easily as they write an email. `seng` aims to become the global standard for introductory programming, serving as a bridge between human language and machine execution. 

In the future, we envision `seng` expanding beyond basic logic to support **AI model building, data science, and complex automation**, allowing users to create sophisticated AI systems using nothing but plain English.

---

## 🚀 Quick Start

```sh
# Run a source file directly
seng hello.se

# Compile to bytecode
seng compile hello.se      # → creates hello.sec

# Run compiled bytecode
seng run hello.sec
```

---

## 📦 Building from Source (requires GCC)

```sh
gcc -std=c99 -O2 -Isrc -o seng \
    src/common.c src/lexer.c src/ast.c src/parser.c \
    src/value.c src/env.c src/interp.c \
    src/compiler.c src/vm.c src/main.c -lm
```

On Windows with MinGW:
```sh
gcc -std=c99 -O2 -Isrc -o seng.exe src/*.c -lm
```

---

## 📖 Language Reference

### Comments
```seng
# This is a comment
note This is also a comment
```

### Variables
```seng
set name to "Alice"
set age to 25
set pi to 3.14
set isReady to true
set empty to nothing
```

### Printing
```seng
say "Hello, World!"
say "Name: " + name
say age
```

### User Input
```seng
ask yourName for "What is your name? "
say "Hello, " + yourName
```

### Arithmetic
```seng
set x to 10 plus 5        # addition:    15
set x to 10 minus 3       # subtraction:  7
set x to 4 * 6            # multiply:    24
set x to 20 divided by 4  # division:     5
set x to 17 mod 3         # modulo:       2
set x to 10 plus 3
```

> **Tip:** Use `+` for string concatenation, `plus`/`minus` for arithmetic keywords, `*`/`/`/`%` as operators.

### Comparisons (inside `if` / `while`)
```seng
age is equal to 25
age is not equal to 30
age is greater than 18
age is less than 65
age is greater than or equal to 18
age is less than or equal to 64
```

### Logical Operators
```seng
if age is greater than 18 and age is less than 65 then ...
if name is equal to "Alice" or name is equal to "Bob" then ...
if not flag then ...
```

### If / Else
```seng
if score is greater than 90 then
    say "A grade"
else if score is greater than 75 then
    say "B grade"
else
    say "Try harder"
end
```

### Repeat Loop
```seng
repeat 5 times
    say "Hello!"
end
```

### While Loop
```seng
set count to 1
while count is less than or equal to 10
    say count
    set count to count plus 1
end
```

### Stop & Skip (break / continue)
```seng
while true
    if x is equal to 5 then
        stop       # break out of loop
    end
    if x mod 2 is equal to 0 then
        skip       # continue to next iteration
    end
    set x to x plus 1
end
```

### Functions
```seng
define greet with name
    say "Hello, " + name + "!"
end

call greet with "Alice"
```

```seng
define addTwo with a and b
    give back a plus b
end

set total to result of addTwo with 10 and 25
say total
```

### Lists
```seng
make list fruits
add "Apple" to fruits
add "Banana" to fruits
add "Cherry" to fruits

say fruits                         # [Apple, Banana, Cherry]
say length of fruits               # 3
say item 1 of fruits               # Apple  (1-indexed)
```

### Import
```seng
import "utils.se"       # runs utils.se in the current scope
```

---

## ⚠️ Reserved Words

The following words are part of the language syntax and cannot be used as variable or function names:

`add` `and` `ask` `back` `by` `call` `define` `divided` `else` `end` `equal` `for` `give` `greater` `if` `import` `is` `item` `length` `less` `list` `make` `minus` `mod` `not` `nothing` `note` `of` `or` `plus` `repeat` `result` `say` `set` `skip` `stop` `than` `then` `times` `to` `while` `with` `true` `false`

---

## 📁 Project Structure

```
seng/
├── src/
│   ├── main.c        Entry point & CLI
│   ├── lexer.c/h     Tokenizer
│   ├── ast.h/c       AST node definitions
│   ├── parser.c/h    Recursive-descent parser
│   ├── value.c/h     Runtime values (num, str, bool, list, func)
│   ├── env.c/h       Variable scoping (hash-map)
│   ├── interp.c/h    Tree-walk interpreter
│   ├── bytecode.h    Bytecode opcode definitions
│   ├── compiler.c/h  AST → .sec bytecode compiler
│   └── vm.c/h        Stack-based virtual machine
├── examples/
│   ├── hello.se
│   ├── loops.se
│   ├── functions.se
│   ├── lists.se
│   └── calculator.se
├── Makefile
└── README.md
```

---

## 🔧 .sec Bytecode Format

Compiled `.sec` files have the following binary format:

| Section      | Format                                       |
|--------------|----------------------------------------------|
| Magic        | `SENG` (4 bytes)                             |
| Version      | `uint8_t` (currently `1`)                    |
| Const pool   | `uint32_t` count + typed entries             |
| Instructions | `uint32_t` count + `(uint8_t op, int32_t arg)` pairs |

---

## 💡 Philosophy

- **Readable**: Code looks like plain English instructions
- **Simple**: No symbols required for common operations  
- **Reusable**: Functions + imports for code sharing
- **Compiled**: Optional `.sec` bytecode for faster distribution

---

## 🤝 Community & Contributing

**seng** is now a public and open-source project! We are building a community to develop and evolve this language. Whether you're a beginner or an expert, we welcome your contributions.

Check out [CONTRIBUTING.md](CONTRIBUTING.md) to get started.

## 💖 Support the Project

If you find **seng** useful, please consider supporting its development through [GitHub Sponsors](https://github.com/sponsors/KANAGARAJ-M). Your support helps us maintain the project and build new features.

## ⚖️ Code of Conduct

We are committed to fostering a welcoming and inclusive community. Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before participating.

---

*seng v1.0.0 — NoCorps.org build by KANAGARAJ-M*
