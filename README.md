<a href="https://liberapay.com/basic-gongfu/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>

## cixl
#### a C-powered, minimalistic extension language

```
cixl v0.1

Press Return twice to eval input.

> 1 2 3
..
[1 2 3]

> 4 5 @
..
[1 2 3 4 5 5]

> _
..
[1 2 3 4 5]

> (6 7 8)
..
[1 2 3 4 5 8]

> !
..
[]

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

> let: foo 42;
..
[]

> $foo
..
[42]

> {1 2 3}
..
[Lambda(0x52d97d0:1)]

> call
..
[1 2 3]
```

### License
GPLv3