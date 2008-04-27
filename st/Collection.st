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

!Collection class methodsFor: 'instance creation'!

new: count
    ^self basicNew: count
!

new
    ^self new: 0
!

with: anObject
    "Answer a collection with a single element"
    ^self new add: anObject; yourself
!
 
with: firstObject with: secondObject
    "Answer a collection with the two given objects"
    ^self new add: firstObject; add: secondObject; yourself
!

withAll: aCollection
    "Create a collection whose elements are the same of aCollection"
    ^self new addAll: aCollection
! !

!Collection methodsFor: 'extensibility'!

growTo: anInteger
    self basicGrowTo: anInteger
!

growBy: anInteger
    self growTo: self size + anInteger
!

add: anObject
    self subclassResponsibility
!

addAll: aCollection
    aCollection do: [ :ea |
	self add: ea ]
!

remove: anObject ifAbsent: exceptionHandler
    "Remove anObject from the collection. Evaluate exceptionHandler if anObject is not found"
    self subclassResponsibility
!

remove: anObject
    "Remove anObject and fail if it's absent"
    self remove: anObject ifAbsent: [ NotFound signal ]
! !

!Collection methodsFor: 'copying'!

copyEmpty
    ^self copyEmpty: self basicSize
!

copyEmpty: newSize
    "Create the empty collection of the given size"
    ^self class new: newSize
!

copyWith: newElement
    "Answer a collection which is a copy of the receiver, and adds a newElement"
    ^(self class withAll: self) add: newElement; yourself
! !

!Collection methodsFor: 'enumerating'!

do: aBlock
    "Call aBlock for each element of the collection"
    self subclassResponsibility
!

collect: transformerBlock
    "Evaluate the block for each element of the receiver.
     The results of these evaluations are collected into a new collection"
    | coll |
    coll := self copyEmpty.
    self do: [ :ea |
	coll add: (transformerBlock value: ea) ].
    ^coll
!

includes: anObject
    "Return true if the receiver contains anObject, otherwise false"
    self do: [ :ea |
	ea = anObject
	    ifTrue: [ ^true ] ].
    ^false
!

occurrencesOf: anObject
    "Answer the number of times anObject is present in the collection"
    | occurs |
    occurs := 0.
    self do: [ :ea |
	ea = anObject
	    ifTrue: [ occurs := occurs + 1 ] ].
    ^occurs
!

reject: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return false"
    | coll |
    coll := self copyEmpty.
    self do: [ :ea |
	(discriminatorBlock value: ea)
	    ifFalse: [ coll add: ea ] ].
    ^coll
!

select: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return true"
    | coll |
    coll := self copyEmpty.
    self do: [ :ea |
	(discriminatorBlock value: ea)
	    ifTrue: [ coll add: ea ] ].
    ^coll
!

detect: discriminatorBlock
    "Evaluate the block for each element of the receiver.
     Answer the first element that caused the block to return true.
     If no element is found, raise an exception"
    self do: [ :ea |
	| res |
	res := discriminatorBlock value: ea.
	res ifTrue: [ ^ea ] ].
    NotFound signal
!

detect: discriminatorBlock ifNone: exceptionHandler
    "Evaluate the block for each element of the receiver.
     Answer the first element that caused the block to return true.
     Evaluate exceptionHandler if any error occurs"
    [ self detect: discriminatorBlock ]
	on: Error do: exceptionHandler
! !
   
!Collection methodsFor: 'basic'!

size
    | count |
    count := 0.
    self do: [ :element | count := count + 1].
    ^count
! !
 
!Collection methodsFor: 'converting'!

asArray
    ^Array withAll: self
!

asSet
    ^Set withAll: self
!

asBag
    ^Bag withAll: self
!

asString
    ^String withAll: self
!

asByteArray
    ^ByteArray withAll: self
!

asOrderedCollection
    ^OrderedCollection withAll: self
! !

!Collection methodsFor: 'testing'!

isEmpty
    ^self size = 0
!

notEmpty
    ^self isEmpty not
! !

