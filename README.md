# SWITCH

Switch is an experiment in using ANSI C to target ANSI C switch statements and gotos.

## Structure
switch.c:

    A recursive descent compiler which parses switch (.sw) files.
    The language of a switch file is reminiscent to the B programming language where
    `int` (and pointers thereto) are the only types.

examples.sw:

    Various examples exemplifying Switch as a language.

tests.sw:

    Unit tests, tests operators and basic expressions and statements.

## Use

run `make` to build `switch`, which will the use `switch` to compile `.sw` files into intermediate `.c`
files that `gcc` then compiles into native executables. The `makefile` will also output the corresponding
`.asm` of each executable for fun.

An example (see more in examples.sw) of a switch program:

```
int factorial(int x)
{
    if(x == 0)
    {
        ret 1;
    }
    else
    {
        ret x * factorial(x - 1);
    }
}

int main()
{
    if(factorial(9) == 362880)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}
```
The output generates an intermediate (.c) that when (post processed)
translates directly to a stack machine that abuses gotos for branching
and switch statements for calling functions returning from functions:

```
int main(void) {
 int fs[4096] = { 0 };
 int bs[4096] = { 0 };
 int vs[65536] = { 0 };
 register int fp = 1;
 register int fa = 0;
 register int sp = 0;
 register int rr = 0;
 goto main;
begin:
 if(fp == 0)
  return vs[sp - 1];
 switch(fa)
 {
factorial:
 vs[sp++] = 0 + bs[fp - 1];
 vs[sp - 1] = vs[vs[sp - 1]];
 vs[sp++] = 0;
 vs[sp - 2] = vs[sp - 2] == vs[sp - 1]; --sp;
 if(vs[--sp] == 0) goto L0;
 vs[sp++] = 1;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
 goto L1;
L0:
 vs[sp++] = 0 + bs[fp - 1];
 vs[sp - 1] = vs[vs[sp - 1]];
 bs[fp] = sp;
 vs[sp++] = 0 + bs[fp - 1];
 vs[sp - 1] = vs[vs[sp - 1]];
 vs[sp++] = 1;
 vs[sp - 2] -= vs[sp - 1]; --sp;
 fs[fp] = 64;fp++;goto factorial;case 64:;
 vs[sp - 2] *= vs[sp - 1]; --sp;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
L1:
 --sp;
 vs[sp++] = 0;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
main:
 bs[fp] = sp;
 vs[sp++] = 9;
 fs[fp] = 74;fp++;goto factorial;case 74:;
 vs[sp++] = 362880;
 vs[sp - 2] = vs[sp - 2] == vs[sp - 1]; --sp;
 if(vs[--sp] == 0) goto L2;
 vs[sp++] = 0;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
 goto L3;
L2:
 vs[sp++] = 1;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
L3:
 vs[sp++] = 0;
 rr = vs[sp - 1];--fp;sp = bs[fp];fa = fs[fp];vs[sp] = rr;sp++;goto begin;
 }
 return 0;
}
```
