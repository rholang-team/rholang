# Milestone 1

## Compiler

Passing small tests, general fuzzing of the compiler to check that it does not respond with errors on valid code.

Minimal required test suite:

Frontend -- parse and check AST correctness for code with extraordinary constructions like:
- deeply nested operators
- assignment into a field of a field of a field of ... of an object (deeply nested reference)
- if-else chains and mixes
- compilcated method calls (call to a method of a field of a field of result of function called on ...)

Translator -- check correctness of IR built from cases listed above, with more thorough cheking of control flow and writing into fields with complicated accessor.

## Runtime

### Main allocator
- sequentially allocate/deallocate objects without corrupting data or headers: allocate/deallocate 10000 objects of random size, check invariants.
- allocate/deallocate objects in random order (fuzzing) without corrupting data or headers: allocate/deallocate randomly 10000 objects, check invariants.

### GC
- mark trees: run mark on some trees, check whether everything alive is marked.
- mark cycles: run mark on cyclic graphs, check whether everything alive is marked.
- sweep big: allocate 10000 objects(dead by default) of random size, then sweep them. check that they are deallocated.
- sweep fast: allocate a random amount of objects of random sizes, then sweep, repeat for 1000 times.
- sweep fuzz: sweep fast, but marking random objects live.