<a href="https://liberapay.com/basic-gongfu/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>

## cixl
#### a minimalistic scripting language

This project aims to produce a minimalistic scripting language, or DSL substrate; in C. In a way, it's Lua taken one step further down the path of simplicity. The implementation is a hybrid interpreter/vm design, designed to be as fast as possible without compromising on simplicity, transparency and flexibility.

### Status
Examples from this document should work without a hitch and run clean in ```valgrind```, outside of that I can't really promise much at the moment. Current work is focused on tracing interactions between core features; as soon as I gain enough confidence that I've steered clear of major disasters, I'll start filling in obvious gaps in functionality.

### Getting Started
To get started, you'll need a decent C compiler with GNU-extensions and CMake installed. A primitive REPL is included, the executable weighs in at 200k. It's highly recommended to run the REPL through ```rlwrap``` for a less nerve-wrecking editing experience.

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

### Types
Any name that starts with an uppercase letter is treated as a type reference. The following types are defined out of the box (with parent types in parens):

- A
- Bool (A)
- Coro (A)
- Func (A)
- Int  (A)
- Lambda (A)
- Str (A)
- Type (A)

```
> type 42
[Int]

> is? A
[t]
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

### Equality
Two flavors of equality are provided.

Value equality:
```
> 'foo' = 'foo'
..
[t]
```

And identity:
```
> 'foo' == 'foo'
..
[f]
```

```
> 42 == 42
..
[t]
```

### Scopes
Enclosing code in parens evaluates it in a separate scope with a clean stack. The last value on the stack is automatically returned on scope exit.

```
> (1 2 3)
..
[3]
```

Variables in the parent scope may be referenced from within the scope, but variables defined inside are not visible from the outside.

```
> let: foo 1;
..(let: foo 2; $foo)
..$foo
..
[2 1]
```


### Lambdas
Putting braces around a block of code defines a lambda, which is then pushed on the stack.

```
> {1 2 3}
..
[Lambda(0x52d97d0:1)]

> call
..
[1 2 3]
```

Lambdas inherit the defining scope.

```
> (let: x 42; {$x}) call
..
[42]
```

### Functions
The ```func:``` macro may be used to define named functions. Several implementations may be defined for the same name as long as they have the same arity and different argument types. Each function opens an implicit scope that is closed on exit.

```
> func: foo() 42;
..foo
..
[42]
```

Prefixing a function name with ```&``` pushes a reference on the stack.

```
> func: foo() 42;
..&foo
..
[Func(foo)]

> call
..
[42]
```

Each argument needs a type, ```A``` may be used to allow any type.

```
> func: bar(x A) $x + 35;
..bar 7
..
[42]
```

Several parameters may share the same type. An index may may be specified instead of type to refer to previous arguments, it is substituted for the actual type on evaluation.

```
> func: baz(x y Int z 0)
    $x + $y + $z;
..baz 1 3 5
..
[9]
```

### Coroutines
Coroutines allow stopping execution and resuming in the same scope later on. A coroutine context is returned on first yield, and ```call```-ing it picks up the trail from previous ```yield```.

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
```

Since functions open implicit scopes, yielding works the same way as for explicit scopes.

```
> func: foo()
..  1 2 yield 3;
..foo call
..
[2 3]
```

Yielding from a lambda inherits the lambda definition scope.

```
> (let: x 42; {yield $x}) call
..
[Coro(0x5476c50:1)]

> call
..
[42]
```

Yielding from main also works. In the example below, ```foo``` manipulates the main stack through the passed in coroutine.

```
> 1 2 yield cls 3 4
..
[1 2 Coro(0x541ec00:1)]

> func: foo(x Coro) $x call;          
..
[1 2 Coro(0x541ec00:1)]

> foo
..
[3 4]
````

### License
GPLv3

Give me a yell if something is unclear, wrong or missing. And please do consider helping out with a [donation](https://liberapay.com/basic-gongfu/donate) if you find this useful, every contribution counts.