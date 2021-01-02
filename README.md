# artin
> A collection of matrix operations over the field of rationals.

## How to Use

Artin operates according to commandline arguments. These are:

| Args | Description |
| ------- | -------- |
| `-a <matrix1> <matrix2>` | Add `matrix1` and `matrix2` together in the typical way, and then print the result to `stdout`.|
| `-m <matrix1> <matrix2>` | Multiply `matrix1` and `matrix2` together in the typical way, and then print the result to `stdout`.|
| `-i <matrix>` | Compute the inverse of `matrix` and then print the result to `stdout`. |

### Examples
```
$ ./artin -m matrices/3x4-1.mat matrices/4x3-1.mat 
3x3
54.000000,98.000000,142.000000,
246.000000,290.000000,334.000000,
438.000000,482.000000,526.000000,
$
$ ./artin -i matrices/2x2-1.mat
2x2
-0.375000,0.500000,
0.625000,-0.500000,
```

## How to Build
The Makefile is configured to build the program. All you need to do is type
```shell script
$ make
```
in the `artin/` directory. This will create the `artin` executable in `artin/`.
