# Changelog

All notable changes to the **seng** programming language are documented here.

Format: `[Version] — YYYY-MM-DD`

---

## [1.0.0] — 2026-04-27

### 🎉 Initial Release

This is the first official public release of **seng — Simple English Programming Language**.

#### Language Features
- **Variables** — `set name to "Alice"`, `set age to 25`, `set active to true`
- **Arithmetic** — `plus`, `minus`, `times` (`*`), `divided by`, `mod`
- **Comparison** — `is equal to`, `is greater than`, `is less than`, `is not equal to`, and combined forms
- **Logical** — `and`, `or`, `not`
- **Output** — `say "Hello, World!"`
- **Input** — `ask name for "Enter your name: "`
- **Conditions** — `if … then … else if … else … end`
- **Loops** — `repeat N times … end` and `while condition … end`
- **Loop control** — `stop` (break), `skip` (continue)
- **Functions** — `define greet with name … end`, `call greet with "Alice"`, `give back value`, `result of func with args`
- **Lists** — `make list fruits`, `add "Apple" to fruits`, `item 1 of fruits`, `length of fruits`
- **Imports** — `import "utils.se"`
- **Comments** — `# this is a comment`
- **Types** — numbers (int/float), strings, booleans (`true`/`false`), `nothing` (null)

#### Tooling
- **Interpreter** — `seng file.se` — run source directly
- **Compiler** — `seng compile file.se` — compile to `.sec` bytecode
- **VM Runner** — `seng run file.sec` — execute bytecode
- **Help** — `seng help`

#### Compiler & Runtime
- Written in **C99** with zero runtime dependencies (only libc + libm)
- Recursive-descent **parser** with line-number error tracking
- **Tree-walk interpreter** for direct execution
- **Stack-based VM** executing custom `.sec` bytecode format
- Reference-counting memory management for strings, lists, and functions
- Lexical scoping with transparent variable mutation across blocks

#### Windows Installer
- Inno Setup-based `.exe` installer (`seng-setup-1.0.0-windows-x64.exe`)
- License acceptance screen (full EULA)
- Selectable install directory
- Automatic system/user PATH configuration
- `.se` and `.sec` file association
- Start Menu shortcuts
- Clean uninstaller

#### Website
- Official site: **nocorps.org/seng**
- Dark-mode landing page with interactive syntax explorer
- Licence-acceptance download modal (scroll → accept → download)
- Platform availability banner (Windows ✓, Linux 🔜, macOS 🔜)

#### Known Limitations (v1.0.0)
- Windows only (Linux and macOS support planned)
- No closures — functions capture their definition scope but do not form true closures
- No string escape sequences beyond basic `\n`, `\t`
- No standard library (file I/O, networking, etc.) — planned for v1.1.0
- Error messages show line numbers but not column positions

---

## [Upcoming] — v1.1.0

> These features are planned and subject to change.

- [ ] Linux and macOS official support
- [ ] Standard library: file I/O (`read file`, `write file`)
- [ ] String methods: `length of`, `upper of`, `lower of`, `contains`
- [ ] `for each item in list` loop syntax
- [ ] Column-level error messages
- [ ] REPL (interactive mode): `seng`
- [ ] VS Code syntax highlighting extension

---

*Maintained by NoCorps.org — [nocorps.org/seng](https://nocorps.org/seng)*
