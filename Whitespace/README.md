## Whitespace

This is a simple **Whitespace interpreter** written in C. Whitespace is an esoteric programming language created 2003 by **Edwin Brady** and **Chris Morris**.

For more information about Whitespace, visit [Wikipedia](https://en.wikipedia.org/wiki/Whitespace_(programming_language)) or [Esolang](https://esolangs.org/wiki/Whitespace).

This part of the Esoteric-Languages repository was finished 19-07-24 by Anton Simmerle. View [the collection](https://github.com/antonsimmerle/Esoteric-Languages).

### Requirements: 📋
* GCC (GNU Compiler Collection)
* Standard C Library (libc)

### Compatibility: 🌐
This program uses only standard C libraries such as 'stdio.h', 'stdlib.h' and 'limits.h', so it should be compatible with UNIX and non UNIX systems.

### Limits: 🔒
This program uses non-exponential dynamic memory allocation for basically everything, including the operator amount, heaps and stacks. The binary values are limited by the C standart int limit.

### Usage: ⚙️
```bash
gcc -o main main.c
./main <input.ws>
```