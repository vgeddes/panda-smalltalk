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

!ByteArray class methodsFor: 'instance creation'!

new: size
    <primitive: 'ByteArray_newColon'>
	self primitiveFailed
!

basicNew: size
    <primitive: 'ByteArray_newColon'>
	self primitiveFailed
! !

!ByteArray methodsFor: 'basic'!

at: anIndex
    <primitive: 'ByteArray_at'>
	self primitiveFailed
!

at: anIndex put: aSmallInteger
    <primitive: 'ByteArray_at_put'>
	self primitiveFailed
!

basicAt: anIndex
    <primitive: 'ByteArray_at'>
	self primitiveFailed
!

basicAt: anIndex put: aSmallInteger
    <primitive: 'ByteArray_at_put'>
	self primitiveFailed
!

byteAt: anIndex
    <primitive: 'ByteArray_at'>
	self primitiveFailed
!

byteAt: anIndex put: aSmallInteger
    <primitive: 'ByteArray_at_put'>
	self primitiveFailed
! !

!ByteArray methodsFor: 'replacing'!

replaceFrom: startIndex to: stopIndex with: aCollection
    <primitive: 'ArrayedCollection_replaceFromToWith'>
	| element j |
	j := 0.
	startIndex to: (stopIndex min: aCollection size) do: [ :i |
	    element := aCollection at: (j := j + 1).
	    (element isCharacter or: [ element isSmallInteger & (element >= 0) & (element <= 255) ])
		ifTrue: [ self at: i put: element ]
		ifFalse: [ self primitiveFailed ] ]		
! !
