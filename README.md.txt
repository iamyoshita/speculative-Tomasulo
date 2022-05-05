# CA_CS2410_proj

## Instructions to execute- works on elements.cs.pitt.edu:
$make clean

$make

$./simulator Test1.dat

## Input instruction:
Memory content followed by space followed by instructions.

The registers for fadd,fmul,fdiv must be float registers.
The destination registers for fld,fst, must be float registers
bne checks only int registers.

Example format Test1.dat: 
```
0, 111
8, 14
16, 5
24, 10
100, 2
108, 27
116, 3
124, 8
200, 12

addi R1, R0, 24
addi R2, R0, 124
fld F2, 200(R0)
loop: fld F0, 0(R1)
addi R1, R1, -8
fmul F0, F0, F2
fld F4, 0(R2)
fadd F0, F0, F4
fsd F0, 0(R2)
addi R2, R2, -8
bne R1, $0, loop
```

