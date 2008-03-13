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

!Number class methodsFor: 'instance creation'!

new
    ^self shouldNotImplement
! !

!Number methodsFor: 'testing'!

isInteger
    ^self subclassResponsibility
!

isZero
    ^self = self zero
!

isFloat
    ^self subclassResponsibility
!

negative
    ^self < self zero
!

positive
    ^self >= self zero
!

sign
    self < self zero
	ifTrue: [ ^-1 ].
    self > self zero
	ifTrue: [ ^1 ].
    ^0
!

isSmallInteger
    ^self subclassResponsibility
! !

!Number methodsFor: 'iterators'!

to: stop do: aBlock
    "Do aBlock from self to stop. Pass the counter to aBlock"
    self to: stop by: self unity do: aBlock
!

to: stop reverseDo: aBlock
    "Do aBlock decreasing self to stop. Pass the counter to aBlock"
    self to: stop by: self unity reverseDo: aBlock
!

to: stop by: step reverseDo: aBlock
    "Do aBlock decreasing self to stop stepping by step. Pass the counter to aBlock"
    | i |
    i := self.
    [ i >= stop ]
	whileTrue: [ aBlock value: i.
		     i := i - step ]
!

to: stop by: step do: aBlock
    "Do aBlock increasing self to stop stepping by step. Pass the counter to aBlock"
    | i |
    i := self.
    [ i <= stop ]
	whileTrue: [ aBlock value: i.
                     i := i + step ]
! !

!Number methodsFor: 'printing'!

printNl
    self print
! !

!Number methodsFor: 'coercing'!

zero
    ^self subclassResponsibility
!

coerce: aNumber
    ^aNumber
! !

!Number methodsFor: 'shortcuts'!

between: firstNumber and: secondNumber
    ^(self >= firstNumber) & (self <= secondNumber)
! !

!Number methodsFor: 'arithmetic'!

negated
    self ~= self zero
	ifTrue: [ ^self zero - self ]
!

abs
    self < self zero
	ifTrue: [ ^self negated ]
!

+ aNumber
     "Coerce aNumber and do the sum"
     self generality > aNumber generality
	 ifTrue: [ ^self + (self coerce: aNumber) ]
	 ifFalse: [ ^(aNumber coerce: self) + aNumber ]
!

- aNumber
    "Coerce aNumber and do the difference"
    self generality > aNumber generality
	ifTrue: [ ^self - (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) - aNumber ]
!

* aNumber
    "Coerce aNumber and do multiplication"
    self generality > aNumber generality
	ifTrue: [ ^self * (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) * aNumber ]
!

< aNumber
    "Coerce aNumber and compare"
    self generality > aNumber generality
	ifTrue: [ ^self < (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) < aNumber ]
!

> aNumber
    "Coerce aNumber and compare"
    self generality > aNumber generality
	ifTrue: [ ^self > (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) > aNumber ]
!

<= aNumber
    "Coerce aNumber and compare"
    self generality > aNumber generality
	ifTrue: [ ^self <= (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) <= aNumber ]
!

>= aNumber
    "Coerce aNumber and compare"
    self generality > aNumber generality
	ifTrue: [ ^self >= (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) >= aNumber ]
!

= aNumber
    "Coerce aNumber and compare equality"
    self generality > aNumber generality
	ifTrue: [ ^self = (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) = aNumber ]
!

== aNumber
     "Coerce aNumber and compare equality"
     self generality > aNumber generality
	 ifTrue: [ ^self == (self coerce: aNumber) ]
	 ifFalse: [ ^(aNumber coerce: self) == aNumber ]
!

~= aNumber
    "Coerce aNumber and compare inequality"
    self generality > aNumber generality
	ifTrue: [ ^self ~= (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) ~= aNumber ]
!

~~ aNumber
    "Coerce aNumber and compare inequality"
    self generality > aNumber generality
	ifTrue: [ ^self ~~ (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) ~~ aNumber ]
!

\\ aNumber
    "Coerce aNumber and do modulo"
    self generality > aNumber generality
	ifTrue: [ ^self \\ (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) \\ aNumber ]
!

quo: aNumber
    "Coerce and answer the truncated quotient"
    self generality > aNumber generality
	ifTrue: [ ^self quo: (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) quo: aNumber ]
!

generality
    self subclassResponsibility
!

unity
    self subclassResponsibility
! !
