# C++23 Shell

A POSIX-compliant shell implementation in C++23, built as part of the [CodeCrafters](https://codecrafters.io/) challenge.

## Features

- **Builtins**: `exit`, `echo`, `type`, `pwd`, `cd`, `history`
- **Pipes**: Full pipeline support (`cmd1 | cmd2 | cmd3`)
- **Redirections**: `>`, `>>`, `2>`, `2>>`, `1>`, `1>>`
- **Tab completion**: Trie-based command completion
- **History**: Persistent history with readline integration

## Shell concepts

| Concept | Implementation |
|---------|----------------|
| Pipeline execution | `fork()` + `pipe()` + `dup2()` chaining |
| File redirections | RAII guard with FD save/restore |
| PATH resolution | Linear search with `access(X_OK)` |
| Quoting | State machine (single/double quotes, escapes) |
| History | readline API with file persistence |

## C++23 highlights

- `std::optional<T>` for nullable values (PATH lookups, pipe FDs, Trie navigation)
- Structured bindings in range-based loops
- Designated initializers for `Redirection` struct
- `std::unique_ptr` with custom deleter for readline memory
- `std::string_view` for zero-copy string operations

## Notable design patterns

### Trie for tab completion

Prefix tree storing all builtins and PATH executables. 

**Space complexity:** O(K × M × N) where K = average children per node, M = average string length, N = stored words.

```
insert("echo"), insert("exit"), insert("env")

       (root)
         |
         e
        / \
       c   x/n
       |    |
       h    i/v
       |    |
       o    t
```

For sparse Tries (few children per node, or large alphabets like Unicode) unordered_map is more space-efficient
For dense Tries (most nodes have many children, small alphabet) array-based approach may be more space-efficient despite wasted slots

### RedirectionGuard (RAII)

Header-only class managing file descriptor redirections:
- Saves original FDs via `dup()` on construction
- Opens target files with appropriate flags
- Redirects via `dup2()`
- Restores original FDs on destruction (exception-safe)

### Pipeline Execution

```
cmd1 | cmd2 | cmd3

[cmd1] --pipe--> [cmd2] --pipe--> [cmd3]
  |                |                |
fork+exec      fork+exec        fork+exec
  stdout→pipe    stdin←pipe      stdin←pipe
                 stdout→pipe
```

## Dependencies

- CMake 3.13+
- C++23 compiler
- readline library (`libreadline-dev` on Debian/Ubuntu)

## Build

```bash
cmake -B build -S .
cmake --build build
```

## Project Structure

```
src/
├── main.cpp             # REPL loop, readline setup
├── parsing.cpp/h        # Tokenizer with quote handling
├── execution.cpp/h      # fork/exec, pipeline orchestration
├── builtin.cpp/h        # Shell builtins
├── path.cpp/h           # PATH search, home expansion
├── completion.cpp/h     # Trie + readline completion
├── command.h            # ParsedCommand, Redirection types
└── redirection_guard.h  # RAII FD management
```

