

"arithmetic"

LargeInteger method!
+ aNumber
	<primitive: 'LargeInteger_add'>
	^ super + aNumber!

LargeInteger method!
- aNumber
	<primitive: 'LargeInteger_sub'>
	^ super - aNumber!

LargeInteger method!
* aNumber
	<primitive: 'LargeInteger_mul'>
	^ super * aNumber!

LargeInteger method!
/ aNumber
	<primitive: 'LargeInteger_div'>
	aNumber = 0
		ifTrue: [ self error: 'cannot divide by 0' ].
	(aNumber isMemberOf: LargeInteger)
		ifTrue: [ ^ Fraction numerator: self denominator: aNumber ]
		ifFalse: [ ^ super / aNumber ]!

LargeInteger method!
// aNumber
     <primitive: 'LargeInteger_intDiv'>
	^ super // aNumber!

LargeInteger method!
\\ aNumber
	<primitive: 'LargeInteger_mod'>
	^ super \\ aNumber!


"mathematics"

LargeInteger method!
gcd: anInteger
	<primitive: 'LargeInteger_gcd'>
	(anInteger isMemberOf: SmallInteger)
		ifTrue: [^ self gcd: (self coerce: anInteger)]
	    ifFalse: [ self error: 'primitive failed']!

LargeInteger method!
lcm: anInteger
	<primitive: 'LargeInteger_lcm'>
	^ super lcm: anInteger!


"comparing"

LargeInteger method!
< aNumber
     <primitive: 'LargeInteger_lt'>
	 ^ super < aNumber!

LargeInteger method!
> aNumber
     <primitive: 'LargeInteger_gt'>
	 ^ super > aNumber!

LargeInteger method!
<= aNumber
     <primitive: 'LargeInteger_le'>
	 ^ super <= aNumber!

LargeInteger method!
>= aNumber
     <primitive: 'LargeInteger_ge'>
	 ^ super >= aNumber!

LargeInteger method!
= aNumber
     <primitive: 'LargeInteger_eq'>
	 ^ super = aNumber!

LargeInteger method!
~= aNumber
     <primitive: 'LargeInteger_ne'>
	 ^ super ~= aNumber!

LargeInteger method!
hash
     <primitive: 'LargeInteger_hash'>
	 self primitiveFailed!


"bit-wise operations"

LargeInteger method!
bitOr: aNumber
     <primitive: 'LargeInteger_bitOr'>
	 ^ super bitOr: aNumber!

LargeInteger method!
bitAnd: aNumber
     <primitive: 'LargeInteger_bitAnd'>
	 ^ super bitAnd: aNumber!

LargeInteger method!
bitXor: aNumber
     <primitive: 'LargeInteger_bitAnd'>
	 ^ super bitXor: aNumber!

LargeInteger method!
bitShift: aNumber
     <primitive: 'LargeInteger_bitShift'>
	 ^ super bitShift: aNumber!


"coercion"

LargeInteger method!
asFloat
	<primitive: 'LargeInteger_asFloat'>
	self primitiveFailed!

LargeInteger method!
asLargeInteger
    ^ self!

LargeInteger method!
asInteger
    ^ self!

LargeInteger method!
generality
    ^ 20!

LargeInteger method!
zero
    ^ 0 asLargeInteger!

LargeInteger method!
unity
    ^ 1 asLargeInteger!

LargeInteger method!
coerce: aNumber
    ^ aNumber asLargeInteger!


"printing"
LargeInteger method!
printStringBase: anInteger
	<primitive: 'LargeInteger_printStringBase'>
	anInteger isSmallInteger
		ifTrue: [ self error: 'expected a large integer' ]!

LargeInteger method!
printOn: aStream base: anInteger
	aStream nextPutAll: (self printStringBase: anInteger)!
