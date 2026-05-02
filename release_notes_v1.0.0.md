# seng v1.0.0 — Initial Release 🎉

> **seng** (Simple English) — Write code the way you think.
>
> The first public release of the seng programming language. Write programs in plain English — no symbols, no complex syntax.

---

## 🚀 What's in v1.0.0

### Language Features
- **Interpreter** — run `.se` source files directly with `seng file.se`
- **Bytecode Compiler** — compile to `.sec` bytecode for faster distribution (`seng compile file.se`)
- **Virtual Machine** — execute `.sec` bytecode with `seng run file.sec`
- **Variables** — `set name to "Alice"` / `set age to 25`
- **Arithmetic** — `plus`, `minus`, `times`, `divided by`, `mod`
- **Strings** — string values, concatenation with `+`
- **Comparisons** — `is equal to`, `is greater than`, `is less than or equal to`, etc.
- **Conditions** — `if … then / else if / else / end`
- **Loops** — `repeat N times` and `while` loops with `stop` (break) and `skip` (continue)
- **Functions** — `define … with …`, `call … with …`, `give back …`, `result of … with …`
- **Lists** — `make list`, `add … to`, `item N of`, `length of`
- **User Input** — `ask variable for "prompt"`
- **Imports** — `import "other_file.se"` for code reuse
- **Comments** — `# this is a comment`

### Tooling
- **Windows Installer** (`.exe`) — installs seng, adds to system PATH, registers `.se` and `.sec` file associations
- **build.bat** — auto-detects GCC / Clang / MSVC and compiles from source
- **Makefile** — for Linux/macOS builds from source

### Website
- Official site at [nocorps.org/seng](https://nocorps.org/seng)
- Full documentation at [nocorps.org/seng/docs.html](https://nocorps.org/seng/docs.html)
- License-acceptance gated download modal

---

## 📦 Download

| Platform | Download |
|----------|----------|
| **Windows 10/11 (x64)** | `seng-setup-1.0.0-windows-x64.exe` |
| Linux | Coming in v1.2.0 |
| macOS | Coming in v1.2.0 |

> **⚠ Important:** Run the installer as **Administrator** so it can add seng to your system PATH.

---

## ⚡ Quick Start

After installation, open a **new** terminal:

```sh
# Verify installation
seng help

# Run a program
seng hello.se

# Compile to bytecode
seng compile hello.se

# Run compiled bytecode
seng run hello.sec
```

---

## 📝 Example Program

```
# hello.se
set name to "World"
say "Hello, " + name + "!"

repeat 3 times
    say "seng is simple!"
end

define greet with person
    say "Welcome, " + person + "!"
end

call greet with "Alice"
```

---

## 🏗 Build from Source

Requires GCC or Clang:

```sh
# Windows (MinGW/GCC)
gcc -std=c99 -O2 -Isrc -o seng.exe src\common.c src\lexer.c src\ast.c src\parser.c src\value.c src\env.c src\interp.c src\compiler.c src\vm.c src\main.c -lm

# Linux / macOS
gcc -std=c99 -O2 -Isrc -o seng src/*.c -lm
```

---

## 🗺 Roadmap

| Version | Focus | ETA |
|---------|-------|-----|
| v1.1.0 | Standard Library (File I/O, String methods, REPL) | Q3 2026 |
| v1.2.0 | Cross-Platform (Linux, macOS, VS Code extension) | Q4 2026 |
| v2.0.0 | seng Evolved (AI suggestions, Web output, Cloud IDE) | 2027 |

---

## ⚠ Disclaimer

Use of seng is entirely at your own risk. NoCorps.org build by KANAGARAJ-M is not responsible for any damage, data loss, or system failure arising from use of this software. See [LICENSE](LICENSE) for full terms.

---

**Created with ❤️ by [NoCorps.org build by KANAGARAJ-M](https://nocorps.org)**
📧 hello@nocorps.org
🌐 https://nocorps.org/seng
