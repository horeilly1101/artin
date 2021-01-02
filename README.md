# artin
> A collection of matrix operations over the field of rationals.

## How to Use

Artin operates according to commandline arguments. These are:
- `-a <matrix1> <matrix2>` adds `matrix1` and `matrix2` together in the typical way,
and then prints the result to `stdout`.
- `-m <matrix1> <matrix2>` multiplies `matrix1` and `matrix2` together in the 
typical way, and then prints the result to `stdout`.
- `-i <matrix>` computes the inverse of `matrix` and then prints the result to 
`stdout`.

### Examples
```shell script
$ ./artin -m matrices/3x4-1.mat matrices/4x3-1.mat 
$ ./artin -i matrices/2x2-1.mat
```

## How to Build
The Makefile is configured to build the program. All you need to do is type
```shell script
$ make
```
in the `artin/` directory. This will create the `artin` executable in `artin/`.
