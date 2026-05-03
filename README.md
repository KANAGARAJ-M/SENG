<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=gradient&text=seng%20Language&height=200&animation=fadeIn&section=header&fontSize=60&fontAlignY=35" width="100%"/>
</div>

<div align="center">
  <img src="SENG.svg" alt="SENG Logo" width="420">
</div>

<p align="center">
  <a href="https://git.io/typing-svg">
    <img src="https://readme-typing-svg.herokuapp.com?font=Fira+Code&size=25&duration=4000&pause=1000&color=F70000&center=true&vCenter=true&multiline=true&random=false&width=600&height=100&lines=Simple+English+Programming;Logic+Over+Syntax;Write+the+way+you+think" alt="Typing SVG" />
  </a>
</p>

<div align="center">
  <img src="https://img.shields.io/github/actions/workflow/status/KANAGARAJ-M/SENG/build.yml?style=for-the-badge&logo=github-actions&logoColor=white" alt="Build Status" />
  <img src="https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge" alt="License" />
  <img src="https://img.shields.io/github/sponsors/KANAGARAJ-M?color=ea4aaa&style=for-the-badge&logo=github-sponsors&logoColor=white" alt="Sponsors" />
</div>

<div align="center">
  <br>
  <img src="https://komarev.com/ghpvc/?username=KANAGARAJ-M&label=Repo%20Views&color=0e75b6&style=flat" alt="Views" />
  <img src="https://img.shields.io/github/stars/KANAGARAJ-M/SENG?label=Stars&style=social" alt="Stars" />
</div>

---

<h2 align="center">📖 About seng</h2>

**seng** (Simple English) is a programming language designed so that even non-programmers can read, write, and understand code. Its syntax is plain English, focusing on the **logic** of your thoughts rather than the **syntax** of the machine.

<details open>
<summary><b>🌎 Mission & Vision</b></summary>
<br>

- **🧩 Ideology**: Programming is a human right. We focus on computational thinking without abstract symbol barriers.
- **🎯 Mission**: Zero-friction entry point into software development for educators and self-learners.
- **🔭 Vision**: Natural language as the global standard for introductory programming and AI automation.
- **💻 Supported Platforms**: Windows 🪟 & Linux 🐧
</details>

---

---

<h2 align="center">🚀 Quick Start</h2>

```sh
# Start an interactive REPL
seng repl

# Run a source file directly (automatically uses _secache if up-to-date)
seng hello.se

# Compile to bytecode (stored in _secache/ folder)
seng compile hello.se

# Run compiled bytecode explicitly
seng run examples/_secache/hello.sec

# Disassemble bytecode
seng disasm hello.sec
```

---

<h2 align="center">🛠️ Language Reference</h2>

<details>
<summary><b>📝 Basics (Variables, Printing, Input)</b></summary>

```seng
# Variables
set name to "Alice"
set age to 25

# Printing
say "Hello, " + name

# Input
ask yourName for "What is your name? "
say "Hello, " + yourName
```
</details>

<details>
<summary><b>🔢 Arithmetic & Logic</b></summary>

```seng
set x to 10 plus 5        # 15
set x to 4 * 6            # 24
set x to 17 mod 3         # 2

if age is greater than 18 and age is less than 65 then
    say "Adult"
end
```
</details>

<details>
<summary><b>🔁 Control Flow (Loops & If)</b></summary>

```seng
# If / Else
if score is greater than 90 then
    say "A grade"
else
    say "Try harder"
end

# Loops
repeat 5 times
    say "Hello!"
end

while count is less than 10
    set count to count plus 1
end
```
</details>

<details>
<summary><b>📦 Functions & Lists</b></summary>

```seng
# Functions
define greet with name
    say "Hello, " + name + "!"
end
call greet with "Alice"

# Lists
make list fruits
add "Apple" to fruits
say item 1 of fruits
```
</details>

<details>
<summary><b>🏗️ Object-Oriented Programming (Blueprints)</b></summary>

```seng
blueprint Person
    has name
    has age
    hidden has secret

    define init with n and a
        set me of name to n
        set me of age to a
        set me of secret to "hush!"
    end

    define greet
        say "Hello, I am " + me of name
    end
end

instance of Person called alice with "Alice" and 30
call greet of alice
```
</details>

<details>
<summary><b>🛡️ Error Handling</b></summary>

```seng
try
    throw "Something went wrong!"
catch err
    say "Caught error: " + err
end
```
</details>

<details>
<summary><b>📚 Standard Library</b></summary>

```seng
import math
import json
import sys

say result of sqrt with 144
set obj to result of json_parse with "{\"key\": \"val\"}"
say result of args
```
</details>

---

<h2 align="center">📦 Standard Library Packages</h2>

SENG v1.0.2 includes several built-in packages:

- **math** — `sqrt`, `sin`, `cos`, `random`, `pi`, etc.
- **sys** — `args()`, `exit()`, `sleep()`.
- **json** — `json_parse()`, `json_stringify()`.
- **string** — `upper()`, `lower()`, `replace()`, `split()`, `join()`.
- **type** — `type_of()`, `to_str()`, `to_num()`.
- **io** — `read_file()`, `write_file()`, `file_exists()`.

---

<details>
<summary><b>🔒 Encapsulation (Hidden Members)</b></summary>

SENG provides the `hidden` keyword for private class members. These members can only be accessed within the blueprint itself (using `me`).

```seng
create blueprint BankAccount
    hidden has balance
    
    define init with amount
        set balance of me to amount
    end
    
    define getBalance
        say "Balance: " + balance of me
    end
end

create instance of BankAccount called myAcc with 1000
# say balance of myAcc  <-- This would fail!
call getBalance of myAcc
```
</details>

<details>
<summary><b>⚠️ Error Handling (Try/Catch)</b></summary>

Gracefully handle runtime errors using `try`, `catch`, and `throw`.

```seng
try
    say "Processing..."
    throw "Connection failed!"
catch err
    say "Caught error: " + err
end
```
</details>

<details>
<summary><b>🛠️ Standard Library (Packages)</b></summary>

Import built-in functionality for math, sys, and more.

```seng
import math
say result of sqrt of 16  # 4

import sys
say result of args        # List of command line arguments
```
</details>

---

<h2 align="center">👨‍💻 Meet the Developer</h2>

<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=gradient&text=Hi,%20I'm%20KANAGARAJ%20M&height=150&animation=fadeIn&section=header&fontSize=40&fontAlignY=35" width="100%"/>
</div>

```javascript
const KANAGARAJ = {
    location: "India 🇮🇳",
    role: "Fullstack Developer",
    currentFocus: "Building Web3 Future",
    skills: {
        languages: ["Dart", "JavaScript", "Java", "Kotlin"],
        frameworks: ["Flutter", "React", "Express"],
        databases: ["MongoDB", "Firebase"],
        tools: ["Git", "VS Code", "Figma"]
    },
    contact: "mkrcreations.dev@gmail.com"
};
```

<div align="center">
  <h3>🛠️ Tech Stack</h3>
  <img src="https://skillicons.dev/icons?i=flutter,dart,react,nodejs,express,firebase,mongodb,js,java,kotlin,git,figma,vscode&theme=dark" />
</div>

<h2 align="center">🏆 Achievements & Trophies</h2>

<p align="center">
  <img src="https://github-profile-trophy.vercel.app/?username=kanagaraj-m&theme=radical&no-frame=true&no-bg=true&row=1&column=7" width="100%" alt="Trophy" />
</p>

---

<h2 align="center">📊 Project & Profile Stats</h2>

<p align="center">
  <img src="https://github-readme-activity-graph.vercel.app/graph?username=KANAGARAJ-M&theme=react-dark&hide_border=true&area=true" width="95%">
</p>

<p align="center">
  <img src="https://github-stats-alpha.vercel.app/api?username=KANAGARAJ-M&cc=22272e&tc=37BCF6&ic=fff&bc=0000" width="49%" />
  <img src="http://github-profile-summary-cards.vercel.app/api/cards/profile-details?username=KANAGARAJ-M&theme=radical" width="49%" />
</p>

---

<h2 align="center">🤝 Connect With Me</h2>

<p align="center">
  <a href="https://twitter.com/mr_kanagaraj_m">
    <img src="https://img.shields.io/badge/Twitter-%231DA1F2.svg?style=for-the-badge&logo=Twitter&logoColor=white" alt="Twitter" />
  </a>
  <a href="https://www.linkedin.com/in/kanagaraj-m-b86439227/">
    <img src="https://img.shields.io/badge/linkedin-%230077B5.svg?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn" />
  </a>
  <a href="https://www.instagram.com/kanagaraj.m_mkr/">
    <img src="https://img.shields.io/badge/Instagram-%23E4405F.svg?style=for-the-badge&logo=Instagram&logoColor=white" alt="Instagram" />
  </a>
</p>

<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=gradient&height=100&section=footer" width="100%"/>
</div>

<p align="center">
  <i>seng v1.0.0 — NoCorps.org built by KANAGARAJ-M</i>
</p>
