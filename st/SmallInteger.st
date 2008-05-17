"
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


"arithmetic"

SmallInteger method!
+ aNumber
	<primitive: 'SmallInteger_add'>
	self primitiveFailed!

SmallInteger method!
- aNumber
	<primitive: 'SmallInteger_minus'>
	self primitiveFailed!
	
SmallInteger method!
* aNumber
	<primitive: 'SmallInteger_mul'>
	self primitiveFailed!

SmallInteger method!
/ aNumber
	<primitive: 'SmallInteger_mod'>
	self primitiveFailed!

SmallInteger method!
\\ aNumber
	<primitive: 'SmallInteger_mod'>
	self primitiveFailed!

SmallInteger method!
// aNumber
	<primitive: 'SmallInteger_mod'>
	self primitiveFailed!


"comparing"

SmallInteger method!
< aNumber
	<primitive: 'SmallInteger_lt'>
	self primitiveFailed!

SmallInteger method!
> aNumber
	<primitive: 'SmallInteger_gt'>
	self primitiveFailed!

SmallInteger method!
<= aNumber
	<primitive: 'SmallInteger_le'>
	self primitiveFailed!

SmallInteger method!
>= aNumber
	<primitive: 'SmallInteger_ge'>
	self primitiveFailed!

SmallInteger method!
= aNumber
	<primitive: 'SmallInteger_eq'>
	self primitiveFailed!

SmallInteger method!
~= aNumber
	<primitive: 'SmallInteger_ne'>
	self primitiveFailed!

SmallInteger method!
hash
	^ self!


"bit-wise operations"

SmallInteger method!
bitAnd: aNumber
	<primitive: 'SmallInteger_bitAnd'>
	self primitiveFailed!

SmallInteger method!
bitOr: aNumber
	<primitive: 'SmallInteger_bitOr'>
	self primitiveFailed!

SmallInteger method!
bitXor: aNumber
	<primitive: 'SmallInteger_bitXor'>
	self primitiveFailed!

SmallInteger method!
bitShift: aNumber
	<primitive: 'SmallInteger_bitShift'>
	self primitiveFailed!


"mathematics"

SmallInteger method!
fibonacci
	self <= 2
		ifTrue: [ ^ 1 ]
		ifFalse: [ ^ (self - 2) fibonacci + (self - 1) fibonacci ].!

SmallInteger method!
max: aNumber
	(self >= aNumber)
		ifTrue: [ ^ self ]
		ifFalse: [ ^ aNumber ]!

SmallInteger method!
min: aNumber
	(self >= aNumber)
		ifTrue: [ ^ aNumber ]
		ifFalse: [ ^ self ]!


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
generality    
		^ 10!