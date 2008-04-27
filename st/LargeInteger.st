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

!LargeInteger class methodsFor: 'initialize-release'!

initialize
    finalizationRequest := true
! !

!LargeInteger methodsFor: 'finalization'!

clear
    <primitive: 'LargeInteger_clear'>
	self primitiveFailed
!

finalize
    self clear
! !

!LargeInteger methodsFor: 'coercing'!

asLargeInteger
    ^self
!

asInteger
    ^self
!

generality
    ^20
!

zero
    ^0 asLargeInteger
!

unity
    ^1 asLargeInteger
!

coerce: aNumber
    ^aNumber asLargeInteger
! !

!LargeInteger methodsFor: 'testing'!

isSmallInteger
    ^false
!

isLargeInteger
    ^true
! !

!LargeInteger methodsFor: 'printing'!

print
    <primitive: 'LargeInteger_print'>
	self primitiveFailed
! !

!LargeInteger methodsFor: 'arithmetic'!

+ aNumber
     <primitive: 'LargeInteger_plus'>
	 ^super + aNumber
!

- aNumber
     <primitive: 'LargeInteger_minus'>
	 ^super - aNumber
!

< aNumber
     <primitive: 'LargeInteger_lt'>
	 ^super < aNumber
!

> aNumber
     <primitive: 'LargeInteger_gt'>
	 ^super > aNumber
!

<= aNumber
     <primitive: 'LargeInteger_le'>
	 ^super <= aNumber
!

>= aNumber
     <primitive: 'LargeInteger_ge'>
	 ^super >= aNumber
!

= aNumber
     <primitive: 'LargeInteger_eq'>
	 ^super = aNumber
!

== aNumber
     <primitive: 'LargeInteger_eq'>
	 ^super == aNumber
!

~= aNumber
     <primitive: 'LargeInteger_ne'>
	 ^super ~= aNumber
!

~~ aNumber
     <primitive: 'LargeInteger_ne'>
	 ^super ~~ aNumber
!

/ aNumber
     self notYetImplemented
!

* aNumber
     <primitive: 'LargeInteger_mul'>
	 ^super * aNumber
!

\\ aNumber
     <primitive: 'LargeInteger_mod'>
	 ^super \\ aNumber
! !

!LargeInteger methodsFor: 'bit-wise operations'!

bitAnd: aSmallInteger
    <primitive: 'LargeInteger_bitAnd'>
	self primitiveFailed
!

bitOr: aSmallInteger
    <primitive: 'LargeInteger_bitOr'>
	self primitiveFailed
!

bitXor: aSmallInteger
    <primitive: 'LargeInteger_bitXor'>
	self primtiveFailed
!

bitShift: aSmallInteger
    <primitive: 'LargeInteger_bitShift'>
	self primitiveFailed
! !
