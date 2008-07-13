"
Copyright (c) 2008 Vincent Geddes
Copyright (c) 2007 Luca Bruno

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


"instance creation"

SmallInteger classMethod!
new
	self basicNew!

SmallInteger classMethod!
basicNew
	self error: 'SmallIntegers can only be created by performing arithmetic'!


"arithmetic"

SmallInteger method!
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

SmallInteger method!
+ aNumber
	<primitive: 'SmallInteger_add'>
	^ super + aNumber! 

SmallInteger method!
- aNumber
	<primitive: 'SmallInteger_sub'>
	^ super - aNumber!
	
SmallInteger method!
* aNumber
	<primitive: 'SmallInteger_mul'>
	^ super * aNumber!

SmallInteger method!
/ aNumber
	<primitive: 'SmallInteger_div'>
	aNumber = 0
		ifTrue: [ self error: 'cannot divide by 0' ].
	(aNumber isMemberOf: SmallInteger)
		ifTrue: [ ^ Fraction numerator: self denominator: aNumber ]
		ifFalse: [ ^ super / aNumber ]!

SmallInteger method!
// aNumber
	<primitive: 'SmallInteger_intDiv'>
	^ super // aNumber!

SmallInteger method!
\\ aNumber
	<primitive: 'SmallInteger_mod'>
	^ super \\ aNumber!



"comparing"

SmallInteger method!
< aNumber
	<primitive: 'SmallInteger_lt'>
	^ super < aNumber!

SmallInteger method!
> aNumber
	<primitive: 'SmallInteger_gt'>
	^ super > aNumber!

SmallInteger method!
<= aNumber
	<primitive: 'SmallInteger_le'>
	^ super <= aNumber!

SmallInteger method!
>= aNumber
	<primitive: 'SmallInteger_ge'>
	^ super >= aNumber!

SmallInteger method!
= aNumber
	<primitive: 'SmallInteger_eq'>
	^ super = aNumber!

SmallInteger method!
~= aNumber
	<primitive: 'SmallInteger_ne'>
	^ super ~= aNumber!

SmallInteger method!
hash
	^ self!


"bit-wise operations"

SmallInteger method!
bitAnd: aNumber
	<primitive: 'SmallInteger_bitAnd'>
	^ super bitAnd: aNumber!

SmallInteger method!
bitOr: aNumber
	<primitive: 'SmallInteger_bitOr'>
	^ super bitOr: aNumber!

SmallInteger method!
bitXor: aNumber
	<primitive: 'SmallInteger_bitXor'>
	^ super bitXor: aNumber!

SmallInteger method!
bitShift: aNumber
	<primitive: 'SmallInteger_bitShift'>
	^ super bitShift: aNumber!


"mathematics"

SmallInteger method!
fibonacci
	self <= 2
		ifTrue: [^ 1]
		ifFalse: [^ (self - 2) fibonacci + (self - 1) fibonacci].!


"testing"

SmallInteger method!
isSmallInteger
	^ true!

SmallInteger method!
isLargeInteger
	^ false!


"coercing"

SmallInteger method!
asInteger    
	^ self!

SmallInteger method!
asFloat    
	<primitive: 'SmallInteger_asFloat'>
	self primitiveFailed!

SmallInteger method!
asLargeInteger    
	<primitive: 'SmallInteger_asLargeInteger'>
	self primitiveFailed!

SmallInteger method!
generality    
	^ 10!

SmallInteger method!
zero    
	^ 0!

SmallInteger method!
unity    
	^ 1!

"benchmark"
SmallInteger method!
benchmark
| size flags prime k count |
   size := 8190.
   1 to: self do:
       [:iter |
       count := 0.
       flags := (Array new: size) atAllPut: true.
       1 to: size do:
           [:i | (flags at: i) ifTrue:
               [prime := i+1.
               k := i + prime.
               [k <= size] whileTrue:
                   [flags at: k put: false.
                   k := k + prime].
               count := count + 1]]].
	^ count!