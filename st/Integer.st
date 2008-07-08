"
Copyright (c) 2008 Vincent Geddes
Copyright (c) 2007 Luca Bruno

This file is part of Smalltalk YX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

"mathematics"

Integer method!
gcd: aNumber
    "Answer the greatest common divisor"
    | num tmp remainder |
    num := self max: aNumber.
    remainder := self min: aNumber.
    [ remainder isZero ]
	whileFalse: [    
	    tmp := remainder.
	    remainder := num \\ remainder.
	    num := tmp ].
    ^ num abs!

Integer method!
isEven
	^ self \\ 2 = self zero!

Integer method!
isOdd
	^ self isEven not!

"bit-wise operations"

Integer method!
allMask: anInteger
    "Answer true if all 1 bits of the receiver are 1 also in the argument"
    ^ (self bitAnd: anInteger) == anInteger!

Integer method!
anyMask: anInteger
    "Answer true if any of the bits that are 1 in the argument are 1 in the receiver"
    ^(self bitAnd: anInteger) ~~ anInteger!

Integer method!
noMask: anInteger
    "Answer true if none of the bits that are 1 in the argument are 1 in the receiver"
    ^ (self bitAnd: anInteger) == 0!

Integer method!
bitAnd: aNumber
    aNumber isInteger
		ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
		ifTrue: [ ^ self bitAnd: (self coerce: aNumber) ]
		ifFalse: [ ^ (aNumber coerce: self) bitAnd: aNumber ]!

Integer method!
bitOr: aNumber
    aNumber isInteger
		ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
		ifTrue: [ ^ self bitOr: (self coerce: aNumber) ]
		ifFalse: [ ^ (aNumber coerce: self) bitOr: aNumber ]!

Integer method!
bitXor: aNumber
    aNumber isInteger
		ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
		ifTrue: [ ^ self bitXor: (self coerce: aNumber) ]
		ifFalse: [ ^ (aNumber coerce: self) bitXor: aNumber ]!

Integer method!
bitShift: aNumber
    aNumber isInteger
		ifFalse: [ self error: 'An integer is required' ].
   
    ^ self asLargeInteger bitShift: aNumber!

Integer method!
bitAt: index
    "Answer the bit 0 or 1 at the given index"
    ^ (self bitShift: (index - 1) negated) bitAnd: 1!

Integer method!
bitInvert
    "Answer an Integer whose bits are inverted"
    ^ self bitXor: -1!

Integer method!
<< aNumber
    ^ self bitShift: aNumber!

Integer method!
>> aNumber
    ^ self bitShift: aNumber negated!

"printing"

Integer method!
radix: base
    "Returns a string representing the integer with the given base.
     The base must be between 2 and 16"

    | stream num |

    base isInteger & (base >= 2 ) & (base <= 16)
		ifFalse: [ self error: 'Argument must be an integer between 2 and 16' ].
    self isZero
		ifTrue: [ ^ '0' ].

    stream := WriteStream on: (String new: 20).
    num := self abs.

    [ num ~= 0 ]
		whileTrue: [
			| cp |
			cp := $0 value + (num \\ base).
			cp > $9 value
				ifTrue: [ cp := cp + $A value - $9 value - 1 ].
			stream nextPut: (Character characterFor: cp).
			num := num // base ].

    self isNegative
		ifTrue: [ stream nextPut: $- ].

    ^ stream contents reversed!


Integer method!
printStringRadix: base
    | stream |
    stream := WriteStream on: (String new: 20).
    self printOn: stream base: base.
    ^ stream contents!

Integer method!
printOn: aStream
    self storeOn: aStream base: 10!

Integer method!
storeOn: aStream
    self storeOn: aStream base: 10!

Integer method!
printOn: aStream base: base
    aStream nextPutAll: (self radix: base)!

Integer method!
storeOn: aStream base: base
    aStream nextPutAll: (self radix: base)!


"rounding"

Integer method!
ceiling
    ^ self!

Integer method!
floor
    ^ self!

Integer method!
truncated
    ^ self!


"testing"

Integer method!
isInteger
    ^ true!


"enumerating"

Integer method!
timesRepeat: aBlock
    1 to: self do: [ :ea | aBlock value ]!
