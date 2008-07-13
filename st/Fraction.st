"
Copyright (c) 2008 Vincent Geddes
Copyright (c) 2007-2008 Luca Bruno

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

Fraction classMethod!
numerator: numerator denominator: denominator    
	^ self basicNew numerator: numerator denominator: denominator!


"accessing"
Fraction method!
numerator
	^ numerator!

Fraction method!
denominator
	^ denominator!

"arithmetic"

Fraction method!
reduce
    | gcd |

    numerator = 0
		ifTrue: [ ^ 0 ].

    gcd := numerator gcd: denominator.

    numerator := numerator // gcd.
    denominator := denominator // gcd.

    denominator = 1
		ifTrue: [ ^ numerator ].
    numerator = denominator
		ifTrue: [ ^ 1 ].
    ^ self!

Fraction method!
+ aNumber
    aNumber generality > self generality
	ifTrue: [ ^ (aNumber coerce: self) + aNumber ].

    ^ aNumber isFraction
	ifTrue: [
	    (Fraction
		 numerator: (numerator * aNumber denominator) + (denominator * aNumber numerator)
		 denominator: denominator * aNumber denominator) reduce ]
	ifFalse: [
	    (Fraction
		 numerator: numerator + (denominator * aNumber)
		 denominator: denominator) reduce ]!

Fraction method!
- aNumber
    aNumber generality > self generality
	ifTrue: [ ^ (aNumber coerce: self) - aNumber ].

    ^ aNumber isFraction
	ifTrue: [
	    (Fraction
		 numerator: (numerator * aNumber denominator) - (denominator * aNumber numerator)
		 denominator: denominator * aNumber denominator) reduce ]
	ifFalse: [
	    (Fraction
		 numerator: numerator - (denominator * aNumber)
		 denominator: denominator) reduce ]!

Fraction method!
* aNumber
    aNumber generality > self generality
	ifTrue: [ ^(aNumber coerce: self) - aNumber ].

    ^ aNumber isFraction
	ifTrue: [
	    (Fraction
		 numerator: numerator * aNumber numerator
		 denominator: denominator * aNumber denominator) reduce ]
	ifFalse: [
	    (Fraction
		 numerator: numerator * aNumber
		 denominator: denominator) reduce ]!

Fraction method!
/ aNumber
    ^ self * aNumber reciprocal!

Fraction method!
reciprocal
    ^ Fraction numerator: denominator denominator: numerator!

Fraction method!
< aNumber
    ^ (self compare: aNumber) < 0!

Fraction method!
> aNumber
    ^ (self compare: aNumber) > 0!

Fraction method!
<= aNumber
    ^ (self compare: aNumber) <= 0!

Fraction method!
>= aNumber
    ^ (self compare: aNumber) >= 0!

Fraction method!
= aNumber
    ^ (self compare: aNumber) = 0!

Fraction method!
~= aNumber
    ^ (self compare: aNumber) ~= 0!


"comparing"

Fraction method!
compare: aNumber
    | a b |
    aNumber generality > self generality
		ifTrue: [ ^ (aNumber coerce: self) compare: aNumber ].

    ^ aNumber isFraction
		ifTrue: [ | d |
			d := denominator * aNumber denominator.
			a := d * self numerator.
			b := d * aNumber numerator.
			a compare: b ]
		ifFalse: [ numerator compare: denominator * aNumber ]!

Fraction method!
hash
	^ numerator bitXor: denominator!

"testing"

Fraction method!
isFraction
	^ true!


"coercing"

Fraction method!
asFraction
	^ self!

Fraction method!
asFloat
    ^ numerator asFloat / denominator asFloat!

Fraction method!
generality    
		^ 40!

Fraction method!
coerce: aNumber    
	^ Fraction numerator: aNumber asInteger
			   denominator: 1!


"truncation"

Fraction method!
truncated
	^ numerator // denominator!

"printing"

Fraction method!
printOn: aStream base: anInteger
	aStream nextPut: $(.
	aStream nextPutAll: (numerator printStringBase: anInteger).
	aStream nextPut: $/.
	aStream nextPutAll: (denominator printStringBase: anInteger).
	aStream nextPut: $)!

"private"

Fraction method!
numerator: numeratorInteger denominator: denominatorInteger    
	numerator := numeratorInteger.
	denominator := denominatorInteger!

