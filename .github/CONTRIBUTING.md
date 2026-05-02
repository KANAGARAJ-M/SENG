# Contributing to seng

Thank you for your interest in contributing to the **seng** programming language!  
seng is a project by **NoCorps.org build by KANAGARAJ-M** (nocorps.org) dedicated to making programming accessible to everyone.

---

## 🌟 Ways to Contribute

- **Report bugs** — File a bug report using the issue template
- **Request features** — Suggest new language constructs or tools
- **Improve documentation** — Fix typos, improve clarity, add examples
- **Write examples** — Add `.se` programs to the `examples/` folder
- **Fix bugs** — Pick up a bug-labeled issue and submit a fix
- **Add tests** — Help grow the test suite
- **Translate** — Translate documentation into other languages

---

## 🛠️ Development Setup

### Requirements
- Windows 10 / 11 (Linux/macOS support coming soon)
- GCC (MinGW-w64) **or** Clang **or** MSVC

### Build
```batch
build.bat
```
Or manually:
```batch
gcc -std=c99 -O2 -Isrc -o seng.exe src/common.c src/lexer.c src/ast.c src/parser.c src/value.c src/env.c src/interp.c src/compiler.c src/vm.c src/main.c -lm
```

### Test your changes
```batch
seng.exe examples\hello.se
seng.exe examples\loops.se
seng.exe examples\functions.se
seng.exe examples\lists.se
seng.exe examples\calculator.se
```

---

## 📐 Code Style

- **Language**: C99 (no C11/C17 features)
- **Indentation**: 4 spaces (no tabs)
- **Naming**: `snake_case` for functions and variables, `UPPER_CASE` for macros/constants
- **Prefix**: All public functions use the module prefix (e.g. `lexer_`, `env_`, `val_`)
- **Comments**: Use `/* */` for block comments, `//` for inline
- **Line length**: Aim for ≤ 100 characters
- **Header guards**: Use `#ifndef SENG_MODULE_H` style

---

## 📁 Project Structure

```
seng/
├── src/
│   ├── common.h/c      Memory, I/O utilities
│   ├── lexer.h/c       Tokenizer
│   ├── ast.h/c         AST node types
│   ├── parser.h/c      Recursive-descent parser
│   ├── value.h/c       Runtime value types
│   ├── env.h/c         Variable scoping
│   ├── interp.h/c      Tree-walk interpreter
│   ├── bytecode.h      Opcode definitions
│   ├── compiler.h/c    AST → .sec compiler
│   ├── vm.h/c          Stack VM
│   └── main.c          CLI entry point
├── examples/           Example .se programs
├── .github/            Issue templates, workflows
├── build.bat           Windows build script
├── Makefile            GCC/Linux build
├── seng_installer.iss  Inno Setup installer script
└── README.md
```

---

## 🔄 Pull Request Process

1. Fork the repository
2. Create a branch: `feature/my-feature` or `fix/issue-123`
3. Make your changes and test thoroughly
4. Ensure all existing examples still work
5. Submit a pull request using the PR template
6. Address any review comments

---

## 📜 License

By contributing to seng, you agree that your contributions will be licensed
under the same license as the project (see the LICENSE file).

---

*Questions? Visit [nocorps.org](https://nocorps.org)*
