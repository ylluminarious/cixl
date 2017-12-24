<a href="https://liberapay.com/basic-gongfu/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>

## cixl
#### a C-powered, minimalistic extension language

This project aims to produce a minimalistic extension language, or DSL substrate; in C. In a way, it's Lua taken one step further down the path of simplicity. The implementation is a hybrid interpreter/vm design, designed to be as fast as possible without compromising on transparency and flexibility.

### Getting Started
To get started, you'll need a decent C compiler and CMake installed. A primitive REPL is included, the executable weighs in at 200k. It's highly recommended to run the REPL through ```rlwrap``` for a less nerve-wrecking editing experience.

```
git clone https://github.com/basic-gongfu/cixl.git
cd cixl
mkdir build
cd build
cmake ..
rlwrap ./cixl

cixl v0.1

Press Return twice to eval input.

> 1 2 3
..
[1 2 3]

> quit
```

### Stack
The parameter stack is exposed to user code, just like in Forth.

```
> 4 5 dup
..
[1 2 3 4 5 5]

> zap
..
[1 2 3 4 5]

> cls
..
[]
```

### Expressions
But unlike Forth, functions scan forward until enough arguments are on the stack to allow reordering parameters and operations in user code to fit the problem being solved.

```
> 1 + 2
..
[3]

> 1 2 +
..
[3 3]

> + 1 2
..
[6 1 2]

> + +
..
[9]
```

### Variables
Named variables may be bound once per scope using the ```let:``` macro.

```
> let: foo 42;
..
[]

> $foo
..
[42]
```

### Lambdas
Braces quote contained code, which is then pushed on the stack.

```
> {1 2 3}
..
[Lambda(0x52d97d0:1)]

> call
..
[1 2 3]
```

### Scopes
Enclosing code in parens evaluates in a separate scope/stack. Variables in the parent scope may be referenced from within the scope, but variables defined inside are not visible from the outside. The last value on the stack is automatically returned on scope exit.

```
> (1 2 3)
..
[3]

> let: foo 1;
..(let: foo 2; $foo)
..$foo
..
[2 1]
```

### Functions
The ```func:``` macro may be used to define named functions. Several implementations may be defined for the same name as long as they have the same number of arguments and different types. An integer may be specified instead of argument type, which is then substituted for the actual type of that argument on evaluation. Each function opens an implicit scope that is closed on exit.

```
> func: foo() 42;
..foo
..
[42]

> func: bar(x Int) $x + 35;
..cls
..bar 7
..
[42]

> func: baz(x y Int z 0) $x + $y + $z;
..cls
..baz 1 3 5
..
[9]

```

### Coroutines
Coroutines allow stopping and resuming execution of a scope. The coroutine context is returned on first yield, and ```call```-ing it picks up the trail from previous ```yield```.

```
> (1 2 yield 3)
..
[2 Coro(0x53c9de0:1)]

> dup call
..
[2 Coro(0x53c9de0:1) 3]

> zap call
..
Error in row 1, col 5:
Coro is done

> func: foo() 1 2 yield 3; foo call
..
[2 3]
```

### License
GPLv3

Give me a yell if something is unclear, wrong or missing. And please do consider helping out with a [donation](https://liberapay.com/basic-gongfu/donate) if you find this useful, every contribution counts.