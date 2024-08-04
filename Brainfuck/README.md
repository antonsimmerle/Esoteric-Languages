## Brainfuck

This is a simple **brainfuck interpreter** written in C. Brainfuck is an esoteric programming language created 1993 by **Urban Müller**.

For more information about brainfuck, visit [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck), [Esolang](https://esolangs.org/wiki/Brainfuck) or [YouTube](https://youtu.be/hdHjjBS4cs8).

This part of the Esoteric-Languages repository was finished 07-02-24 by Anton Simmerle. View [the collection](https://github.com/antonsimmerle/Esoteric-Languages).

### Requirements: 📋
* GCC (GNU Compiler Collection)
* Standard C Library (libc)

### Compatibility: 🌐
This program uses only standard C libraries such as 'stdio.h' and 'stdlib.h', so it should be compatible with UNIX and non UNIX systems.

### Limits: 🔒
This program uses non-exponential dynamic memory allocation basically everything, including the cell amount, meaning there is no 30000 cell limit. A sequence of equal operators get compressed into a int, therefore is the amount of sequential operators limited by the standart C int limit.

### Usage: ⚙️
```bash
gcc -o main main.c
./main <input.b>
```