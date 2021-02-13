## 1. First just run `checksum.py` with no arguments. Compute the additive, XOR-based, and Fletcher checksums. Use `-c` to check your answers.

> ❯ ./checksum.py
>
> OPTIONS seed 0
> OPTIONS data_size 4
> OPTIONS data
>
> Decimal:          216        194        107         66
> Hex:             0xd8       0xc2       0x6b       0x42
> Bin:       0b11011000 0b11000010 0b01101011 0b01000010
>
> Add:      ?
> Xor:      ?
> Fletcher: ?

Add:      (216 + 194 + 107 + 66) % 256 = 71
Xor:      0b110011 = 51
Fletcher:
216        194        107         66
216        155        7           73
216        116        123         196

(73, 196)

> example from book:
> 1  2  3  4
> 1  3  6  10
> 1  4  10 20

## 2. Now do the same, but vary the seed (-s) to different values.

> ❯ ./checksum.py -s 2
>
> OPTIONS seed 2
> OPTIONS data_size 4
> OPTIONS data
>
> Decimal:          244        242         14         21
> Hex:             0xf4       0xf2       0x0e       0x15
> Bin:       0b11110100 0b11110010 0b00001110 0b00010101
>

Add:      (244 +      242       +  14       + 21) % 256 = 9
Xor:      0b11110100 ^ 0b11110010 ^ 0b00001110 ^ 0b00010101 = 29 (0b11101)
Fletcher:
244        242         14         21
244        231         245        11
244        220         210        221

(11, 221)


## 3. Sometimes the additive and XOR-based checksums produce the same checksum (e.g., if the data value is all zeroes). Can you pass in a 4-byte data value (using the -D flag, e.g.,-D a,b,c,d) that does not contain only zeroes and leads the additive and XOR-based checksum having the same value? In general, when does this occur? Check that you are correct with the -c flag.

eg:
> ./checksum.py -D 85,170,0,0 -c

0b10101010
0b01010101
0b00000000
0b00000000

eg:
> ./checksum.py -D 0,0,15,240 -c
0b00000000
0b00000000
0b00001111
0b11110000

This happens anytime the XOR product and the sum of all base-2 digits are the same.


## 4. Now pass in a 4-byte value that you know will produce a different checksum values for additive and XOR. In general, when does this occur?

This happens anytime the XOR product and the sum of all base-2 digits are NOT the same.

## 5. Use the simulator to compute checksums twice (once each for a different set of numbers). The two number strings should be different (e.g. `,-D a1,b1,c1,d1` the first time and `-D a2,b2,c2,d2` the second) but should produce the same additive checksum. In general, when will the additive checksum be the same, even though the data values are different? Check your specific answer with the -c flag.

Anytime the sum of the numbers has the same mod 256 value.

eg:

The sum is 10:
> ./checksum.py -D 1,2,3,4 -c

The sum is also 10 (order doesn't matter):
> ./checksum.py -D 4,3,2,1 -c

This sum is 266:
> ./checksum.py -D 250,2,2,12 -c

This sum is 522:
> ./checksum.py -D 510,6,3,3 -c

## 6. Now do the same for the XOR checksum.

Anytime the XOR of the numbers has the same value, aka:
anytime the only difference between the data is an even number of 1's in the base-2 column of the input.

eg:

The XOR is 2:
> ./checksum.py -D 36,36,2,0 -c

The XOR is also 2:
> ./checksum.py -D 36,32,4,2 -c

The XOR is also 2 (order doesn't matter):
> ./checksum.py -D 2,4,32,36 -c


## 7. Now let’s look at a specific set of data values.  The first is:`-D 1,2,3,4`. What will the different checksums (additive, XOR, Fletcher) be for this data? Now compare it to computing these checksums over `-D 4,3,2,1`. What do you notice about these three check-sums? How does Fletcher compare to the other two? How is Fletcher generally “better” than something like the simple additive check-sum?

Fletcher checksums do not collide when the order is different. XOR and addition do collide.

## 8. No checksum is perfect. Given a particular input of your choosing, can you find other data values that lead to the same Fletcher checksum? When, in general, does this occur? Start with a simple datastring (e.g., `-D 0,1,2,3`) and see if you can replace one of those numbers but end up with the same Fletcher checksum. As always,use -c to check your answers

> ./checksum.py -D 0,1,2,3 -c

When any of the bits are off by 255, it'll escape the mod 255 operation in the Fletcher algorithm, resulting in the same number:

> ./checksum.py -D 0,1,257,3 -c
